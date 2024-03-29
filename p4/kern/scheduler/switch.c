/** @file switch.c
 *
 *  @brief C functions for context switching
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control_block.h>
#include <switch.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>
#include <stack_info.h>
#include "scheduler_internal.h"
#include "interrupt.h"

/** @brief A struct representing the stack before a context switch */
typedef struct context_stack {
    void* edi;
    void* esi;
    void* ebp;
    void* esp;
    void* ebx;
    void* edx;
    void* ecx;
    void* eax;
    void* func_addr;
    void* dummy;
    void* saved_esp;
} context_stack_t;

/** @brief Stores the saved_esp into the tcb
 *
 *  @param saved_esp Stack address of
 *  @param tcb Pointer to tcb
 *  @return void
 **/
void store_esp(void* saved_esp, tcb_t* tcb)
{
    tcb->saved_esp = saved_esp;
}

/** @brief Context switch from one thread to another
 *
 *  @param from The thread to switch from
 *  @param to The thread to switch to
 *  @return void
 **/
void context_switch(tcb_t* from, tcb_t* to)
{
    switch_ppd(to->process->directory);
    switch_stack_and_regs(to->saved_esp, from);
    enable_interrupts();
}

/** @brief Sets up a given thread stack for entry via context switch
 *
 *  @param tcb Thread whose stack is to be set up for context switch entry
 *  @return void
 **/
void setup_for_switch(tcb_t* tcb)
{
    void* saved_esp = tcb->saved_esp;
    context_stack_t context_stack = {
        .func_addr = go_to_user_mode,
        .saved_esp = saved_esp,
    };
    PUSH_STACK(tcb->saved_esp, context_stack, context_stack_t);
}
