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

int copy_process(tcb_t* tcb_parent, ureg_t* state);
int copy_thread(tcb_t* child, tcb_t* parent, ureg_t* state);

/** @brief Handler function for fork()
 *
 *  @return void
 */
void fork_syscall(ureg_t state)
{
    tcb_t* tcb_parent = get_tcb();
    //cant call fork with more than one thread
    if (get_thread_count(tcb_parent->process) > 1) {
        state.eax = -1;
        return;
    }
    state.eax = copy_process(tcb_parent, &state);
}

void thread_fork_syscall(ureg_t state)
{
    tcb_t* parent = get_tcb();
    pcb_t* process = parent->process;
    mutex_lock(&process->children_mutex);
    tcb_t* child = create_tcb_entry(get_next_id());
    if (child == NULL) {
        mutex_unlock(&process->children_mutex);
        state.eax = -1;
        return;
    }
    state.eax = copy_thread(parent, child, &state);
    mutex_unlock(&process->children_mutex);
}

/** @brief Copies the kernel stack from a parent to child process
 *
 *  @param tcb_parent Pointer to parent tcb
 *  @param tcb_child Pointer to child tcb
 *  @return void
 **/
void copy_kernel_stack(tcb_t* tcb_parent, tcb_t* tcb_child)
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
void copy_saved_esp(tcb_t* parent, tcb_t* child, void* state)
{
    uint32_t offset = (uint32_t)parent->kernel_stack - (uint32_t)state;
    child->saved_esp = (void*)((uint32_t)child->kernel_stack - offset);
}

int copy_thread(tcb_t* child, tcb_t* parent, ureg_t* state)
{
    copy_saved_esp(parent, child, state);
    child->swexn = parent->swexn;
    // Copy kernel stack with return value of 0 for child
    state->eax = 0;
    copy_kernel_stack(parent, child);
    // Setup stack for re-entry via context_switch
    setup_for_switch(child);
    // add child to kernel list of threads
    kernel_add_thread(child);
    // schedule the child thread
    schedule(child);
    return child->id;
}

/** @brief Creates a copy of the given process
 *
 *  @return Pointer to tcb on success, null on failure
 **/
int copy_process(tcb_t* tcb_parent, ureg_t* state)
{
    pcb_t* pcb_parent = tcb_parent->process;

    // Create copy of pcb & tcb
    tcb_t* tcb_child = create_pcb_entry();
    if (tcb_child == NULL) {
        return -1;
    }
    // Copy memory regions
    if (init_ppd_from(&tcb_child->process->directory, &pcb_parent->directory)) {
        pcb_t* proc = tcb_child->process;
        free_tcb(tcb_child);
        free_pcb(proc);
        return -1;
    }
    pcb_add_child(pcb_parent, tcb_child->process);
    // Register child process for simics user space debugging
    sim_reg_child(tcb_child->process->directory.dir, pcb_parent->directory.dir);
    copy_thread(tcb_child, tcb_parent, state);
    return tcb_child->process->id;
}
