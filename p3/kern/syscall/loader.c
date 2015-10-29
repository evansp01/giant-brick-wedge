/** @file loader.c
 *  @brief Implementation of functions to load programs
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>
#include <simics.h>
#include <cr.h>
#include <switch.h>
#include <stdint.h>
#include <seg.h>
#include <eflags.h>
#include <ureg.h>
#include <mode_switch.h>
#include <utilities.h>
#include <vm.h>

#include <asm.h> //temp

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return void
 **/
void *create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip)
{
    uint32_t eflags_start = EFL_RESV1 | EFL_IF;
    ureg_t ureg = {
        .ss = SEGSEL_USER_DS,
        .esp = user_esp,
        .eflags = eflags_start,
        .cs = SEGSEL_USER_CS,
        .eip = user_eip,
        .gs = SEGSEL_USER_DS,
        .fs = SEGSEL_USER_DS,
        .es = SEGSEL_USER_DS,
        .ds = SEGSEL_USER_DS
    };
    void *kernel_stack = (void*)stack;
    PUSH_STACK(kernel_stack, ureg, ureg_t);
    
    return kernel_stack;
}

/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on succes; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
    int i, byte_index;

    for (i = 0; i < exec2obj_userapp_count; i++) {
        if (strcmp(filename, exec2obj_userapp_TOC[i].execname) == 0)
            break;
    }

    // No program matching the given filename found
    if (i == exec2obj_userapp_count)
        return -1;

    // Check if given offset and size exceeds the file size
    if ((offset + size) > exec2obj_userapp_TOC[i].execlen)
        return -1;

    // Copy bytes over to buffer
    for (byte_index = 0; byte_index < size; byte_index++) {
        buf[byte_index] = exec2obj_userapp_TOC[i].execbytes[offset+byte_index];
    }

    return byte_index;
}

/** @brief Creates page directory and copies process data into memory
 *
 *  @param elf Struct containing elf file information
 *  @return page directory of new process
 **/
page_directory_t* create_proc_pagedir(simple_elf_t* elf)
{
    page_directory_t* dir = create_page_directory();
    if (dir == NULL) {
        return NULL;
    }
    set_cr3((uint32_t)dir);

    allocate_pages(dir,(void*)elf->e_txtstart,elf->e_txtlen,e_read_page);
    getbytes(elf->e_fname,elf->e_txtoff,elf->e_txtlen,(char*)elf->e_txtstart);

    allocate_pages(dir,(void*)elf->e_rodatstart,elf->e_rodatlen,e_read_page);
    getbytes(elf->e_fname,elf->e_rodatoff,elf->e_rodatlen,
             (char*)elf->e_rodatstart);

    allocate_pages(dir, (void*)elf->e_datstart, elf->e_datlen, e_write_page);
    getbytes(elf->e_fname,elf->e_datoff,elf->e_datlen,(char*)elf->e_datstart);

    allocate_pages(dir, (void*)elf->e_bssstart, elf->e_bsslen, e_write_page);
    memset((void*)elf->e_bssstart, 0, elf->e_bsslen);

    return dir;
}

/** @brief Creates a new idle process
 *
 *  @return Pointer to tcb on success, null on failure
 **/
tcb_t *create_idle()
{
    tcb_t* tcb_entry = create_pcb_entry(NULL);
    
    if (load_program(tcb_entry, "fork_test1") < 0) {
        lprintf("cannot load program");
        return 0;
    }
    return tcb_entry;
}

/** @brief Creates a copy of the given process
 *
 *  @return Pointer to tcb on success, null on failure
 **/
tcb_t *create_copy(tcb_t *tcb_parent)
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
    calc_saved_esp(tcb_parent, tcb_child);
    
    // Copy memory regions
    if (copy_program(pcb_parent, tcb_child->parent) < 0) {
        lprintf("cannot copy program");
        return 0;
    }
    
    return tcb_child;
}

/** @brief Calculates the saved esp for the new thread stack
 *
 *  @param tcb_parent Pointer to parent tcb
 *  @param tcb_child Pointer to child tcb
 *  @return void
 **/
void calc_saved_esp(tcb_t* tcb_parent, tcb_t *tcb_child)
{
    uint32_t child_mask = (uint32_t)tcb_child->kernel_stack & 0xFFFFF000;
    uint32_t parent_mask = (uint32_t)tcb_parent->saved_esp & 0x00000FFF;
    tcb_child->saved_esp = (void *)(parent_mask | child_mask);
}

/** @brief Allocates the stack for the new process
 *
 *  @param cr2 Address of the page table
 *  @param stack_high Highest address for the new stack
 *  @param argc Number of user arguments
 *  @param argv Pointer to user arguments
 *  @return Pointer to the top of the new stack
 **/
uint32_t setup_argv(void *cr2, uint32_t stack_high, int argc, char** argv)
{
    uint32_t stack_low = 0xFFFFF000;
    uint32_t stack_size = stack_high - stack_low;
    allocate_pages(cr2, (void *)stack_low, stack_size, e_write_page);
    memset((void*)stack_low, 0, stack_size);
    return stack_high;
}

/** @brief Sets up the stack for the new process
 *
 *  @param cr2 Address of the page table
 *  @param argc Number of user arguments
 *  @param argv Pointer to user arguments
 *  @return Pointer to esp for the new stack
 **/
uint32_t setup_main_stack(void *cr2, int argc, char** argv)
{
    uint32_t stack_high = 0xFFFFFFFFF & STACK_ALIGN;
    uint32_t *stack_current = (uint32_t *)setup_argv(cr2,stack_high,argc,argv);
    uint32_t stack_low = 0xFFFFF000;
    stack_current[-1] = stack_low;
    stack_current[-2] = stack_high;
    stack_current[-3] = (uint32_t) stack_current;
    stack_current[-4] = argc;
    stack_current[-5] = 0xDEAD1337;
    return (uint32_t) (stack_current - 6);
}

/** @brief Loads the given program file
 *
 *  @param pcb Process to load program on
 *  @param tcb Thread to load program on
 *  @param filename Name of the program to be loaded
 *  @return Pointer to tcb on success, null on failure
 **/
int load_program(tcb_t* tcb, char* filename)
{
    pcb_t* pcb = tcb->parent;
    
    simple_elf_t elf;
    if (elf_check_header(filename) < 0) {
        lprintf("%s not a process", filename);
        return -1;
    }
    elf_load_helper(&elf, filename);
    pcb->directory = (page_directory_t *)create_proc_pagedir(&elf);
    if (pcb->directory == NULL) {
        panic("directory is empty");
        return -1;
    }
    uint32_t stack_entry = setup_main_stack(pcb->directory, 0, NULL);
    
    // Craft kernel stack contents
    tcb->saved_esp = create_context((uint32_t)tcb->kernel_stack, stack_entry,
                                   elf.e_entry);
    
    return 0;
}

/** @brief Copies the page directory tables into a new process
 *
 *  @param pcb_parent PCB of parent process
 *  @param pcb_child PCB of child process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_program(pcb_t* pcb_parent, pcb_t* pcb_child)
{
    extern kernel_state_t kernel_state;
    page_directory_t* dir_parent = pcb_parent->directory;
    
    // Create page directory for child process
    page_directory_t* dir_child = create_page_directory();
    if (dir_child == NULL) {
        return -1;
    }
    pcb_child->directory = dir_child;
    
    // Temporarily set to kernel identity mapping
    set_cr3((uint32_t)kernel_state.dir);
    pcb_parent->directory = kernel_state.dir;
    
    // Copy page tables from parent to child
    if (copy_page_tables(dir_parent, dir_child) < 0)
        return -2;
    
    // Return to process page directory mapping
    set_cr3((uint32_t)dir_parent);
    pcb_parent->directory = dir_parent;
    
    return 0;
}

/** @brief Copies the page tables into a new process
 *
 *  @param dir_parent PCB of parent process
 *  @param dir_child PCB of child process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_page_tables(page_directory_t* dir_parent, page_directory_t* dir_child)
{
    // Copy memory regions
    int i_dir;
    for (i_dir = 0; i_dir < TABLES_PER_DIR; i_dir++) {
        
        // Check if it is a present user directory entry
        entry_t *dir_entry_parent = &dir_parent->tables[i_dir];
        if ((dir_entry_parent->present)&&(dir_entry_parent->user)) {
            
            // Create copy of page table
            entry_t *dir_entry_child = &dir_child->tables[i_dir];
            void* table = create_page_table();
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
            void* frame = allocate_frame();
            if (frame == NULL) {
                lprintf("Ran out of frames to allocate");
                return -2;
            }
            *table_entry_child = create_entry(frame, *table_entry_parent);
            
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

