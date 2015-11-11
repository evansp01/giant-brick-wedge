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
#include <switch.h>
#include <stdint.h>
#include <ureg.h>
#include <mode_switch.h>
#include <utilities.h>
#include <scheduler.h>
#include <vm.h>
#include <asm.h> //temp
#include <stack_info.h>

tcb_t *create_copy(tcb_t *tcb_parent, ureg_t *state);

/** @brief Handler function for fork()
 *
 *  @return void
 */
void fork_syscall(ureg_t state)
{
    tcb_t* tcb_parent = get_tcb();
    //cant call fork with more than one thread
    if(get_thread_count(tcb_parent->process) > 1){
        state.eax = -1;
        return;
    }

    // Copy memory regions to new memory
    // TODO: refactor into thread_fork and fork parts
    tcb_t* tcb_child = create_copy(tcb_parent, &state);
    if(tcb_child == NULL){
        state.eax = -1;
        return;
    }
    // Copy kernel stack with return value of 0 for child
    // Setup stack for re-entry via context_switch
    setup_for_switch(tcb_child);
    // Schedule the child
    schedule(tcb_child);
    // Register child process for simics user space debugging
    sim_reg_child(tcb_child->process->directory.dir,
                  tcb_parent->process->directory.dir);
    // Return child tid to parent
    lprintf("%d spawned %d", tcb_parent->id, tcb_child->id);
    state.eax = tcb_child->id;

}

/** @brief Copies the kernel stack from a parent to child process
 *
 *  @param tcb_parent Pointer to parent tcb
 *  @param tcb_child Pointer to child tcb
 *  @return void
 **/
void copy_kernel_stack(tcb_t *tcb_parent, tcb_t *tcb_child)
{
    uint32_t child_addr = K_STACK_BASE(tcb_child->kernel_stack);
    uint32_t parent_addr = K_STACK_BASE(tcb_parent->kernel_stack);
    // PAGE_SIZE-8 ensures that the pointer to tcb is not overwritten
    memcpy((void*)child_addr, (void*)parent_addr, K_STACK_SPACE);
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
tcb_t *create_copy(tcb_t *tcb_parent, ureg_t *state)
{
    pcb_t* pcb_parent = tcb_parent->process;

    // Create copy of pcb & tcb
    tcb_t* tcb_child = create_pcb_entry();
    if(tcb_child == NULL){
        return NULL;
    }
    // Copy tcb data
    calc_saved_esp(tcb_parent, tcb_child, state);
    
    // Copy swexn
    tcb_child->swexn = tcb_parent->swexn;

    // Copy memory regions
    if(init_ppd_from(&tcb_child->process->directory, &pcb_parent->directory)){
        pcb_t *proc = tcb_child->process;
        free_tcb(tcb_child);
        free_pcb(proc);
        return NULL;
    }
    // Now that we know things worked, add to lists
    pcb_add_child(pcb_parent, tcb_child->process);
    kernel_add_thread(tcb_child);

    state->eax = 0;
    copy_kernel_stack(tcb_parent, tcb_child);

    return tcb_child;
}
