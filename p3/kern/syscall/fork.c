/** @file fork.c
 *  @brief Implementation of functions to create new process via fork
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <loader.h>
#include <simics.h>
#include <cr.h>
#include <switch.h>
#include <stdint.h>
#include <seg.h>
#include <eflags.h>
#include <ureg.h>
#include <mode_switch.h>
#include <utilities.h>
#include <scheduler.h>
#include <vm.h>
#include <asm.h> //temp


/** @brief Handler function for fork()
 *
 *  @return void
 */
void fork_syscall(ureg_t state)
{
    disable_interrupts();
    tcb_t* tcb_parent = get_tcb();

    // Copy memory regions to new memory
    tcb_t* tcb_child = create_copy(tcb_parent, &state);
    if(tcb_child == NULL){
        state.eax = -1;
        return;
    }

    // Copy kernel stack with return value of 0 for child
    state.eax = 0;
    copy_kernel_stack(tcb_parent, tcb_child);

    // Setup stack for re-entry via context_switch
    setup_for_switch(tcb_child);

    // Schedule the child
    schedule(tcb_child);
    
    // Register child process for simics user space debugging
    sim_reg_child(tcb_child->parent->directory.dir,
                  tcb_parent->parent->directory.dir);

    // TODO: Copy software exception handler (if installed)

    // Return child tid to parent
    state.eax = tcb_child->id;
    enable_interrupts();
}

/** @brief Copies the kernel stack from a parent to child process
 *
 *  @param tcb_parent Pointer to parent tcb
 *  @param tcb_child Pointer to child tcb
 *  @return void
 **/
void copy_kernel_stack(tcb_t *tcb_parent, tcb_t *tcb_child)
{
    uint32_t child_addr = (uint32_t)tcb_child->kernel_stack & 0xFFFFF000;
    uint32_t parent_addr = (uint32_t)tcb_parent->kernel_stack & 0xFFFFF000;
    // PAGE_SIZE-8 ensures that the pointer to tcb is not overwritten
    memcpy((void*)child_addr, (void*)parent_addr, PAGE_SIZE-8);
}


/** @brief Calculates the saved esp for the new thread stack
 *
 *  @param tcb_parent Pointer to parent tcb
 *  @param tcb_child Pointer to child tcb
 *  @return void
 **/
void calc_saved_esp(tcb_t* parent, tcb_t *child, void *state)
{
    uint32_t offset = (uint32_t)parent->kernel_stack - (uint32_t)state;
    child->saved_esp = (void *) ((uint32_t)child->kernel_stack - offset);
}

/** @brief Creates a copy of the given process
 *
 *  @return Pointer to tcb on success, null on failure
 **/
tcb_t *create_copy(tcb_t *tcb_parent, void *state)
{
    pcb_t* pcb_parent = tcb_parent->parent;

    // Reject calls to fork for processes with more than one thread
    if (pcb_parent->num_threads > 1) {
        lprintf("Fork called on task with multiple threads");
        return NULL;
    }

    // Create copy of pcb & tcb
    tcb_t* tcb_child = create_pcb_entry(pcb_parent);

    // Copy tcb data
    calc_saved_esp(tcb_parent, tcb_child, state);

    // Copy memory regions
    if (copy_program(pcb_parent, tcb_child->parent) < 0) {
        lprintf("cannot copy program");
        return 0;
    }

    return tcb_child;
}

/** @brief Copies the page directory tables into a new process
 *
 *  @param pcb_parent PCB of parent process
 *  @param pcb_child PCB of child process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_program(pcb_t* pcb_parent, pcb_t* pcb_child)
{
    if(init_ppd_from(&pcb_child->directory, &pcb_parent->directory)){
        return -1;
    }
    return 0;
}

/** @brief Copies the page tables into a new process
 *
 *  @param dir_child PCB of child process
 *  @param dir_parent PCB of parent process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_page_tables(page_directory_t* dir_child, page_directory_t* dir_parent)
{
    // Copy memory regions
    int i_dir;
    for (i_dir = 0; i_dir < TABLES_PER_DIR; i_dir++) {

        // Check if it is a present user directory entry
        entry_t *dir_entry_parent = &dir_parent->tables[i_dir];
        if ((dir_entry_parent->present)&&(dir_entry_parent->user)) {

            // Create copy of page table
            entry_t *dir_entry_child = &dir_child->tables[i_dir];
            void* table = alloc_page_table();
            if (table == NULL) {
                lprintf("Ran out of kernel memory for page tables");
                return -1;
            }
            *dir_entry_child = create_entry(table, *dir_entry_parent);

            // Get page table
            page_table_t *table_parent = get_entry_address(*dir_entry_parent);
            page_table_t *table_child = get_entry_address(*dir_entry_child);

            // Copy page frames from parent to child
            if (copy_frames(table_parent, table_child) < 0)
                return -2;
        }
    }
    return 0;
}

/** @brief Copies the page frames into a new process
 *
 *  @param table_parent PCB of parent process
 *  @param table_child PCB of child process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_frames(page_table_t *table_parent, page_table_t *table_child)
{
    int i_page;
    for (i_page = 0; i_page < PAGES_PER_TABLE; i_page++) {

        // Check if it is a present user page table entry
        entry_t *table_entry_parent = &table_parent->pages[i_page];
        if ((table_entry_parent->present)&&(table_entry_parent->user)) {

            // Create copy of page
            entry_t *table_entry_child = &table_child->pages[i_page];
            if(kernel_alloc_frame(table_entry_child, *table_entry_parent) < 0){
                lprintf("Ran out of frames to allocate");
                return -2;
            }
            // Copy frame data
            memcpy(get_entry_address(*table_entry_child),
                   get_entry_address(*table_entry_parent), PAGE_SIZE);
        }
    }
    return 0;
}

/** @brief Sets up a given thread stack for entry via context switch
 *
 *  @param tcb Thread whose stack is to be set up for context switch entry
 *  @return void
 **/
void setup_for_switch(tcb_t *tcb)
{
    void *saved_esp = tcb->saved_esp;

    context_stack_t context_stack = {
        .func_addr = first_entry_user_mode,
        .saved_esp = saved_esp,
    };

    PUSH_STACK(tcb->saved_esp, context_stack, context_stack_t);
}

