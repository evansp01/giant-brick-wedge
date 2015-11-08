/** @file switch.c
 *
 *  @brief C functions for context switching
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control.h>
#include <switch.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>
#include <loader.h>
#include <mode_switch.h>
#include <stack_info.h>

/* --- Structs --- */
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
 *  @param addr Stack address of 
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
    scheduler_pre_switch(from, to);
    switch_stack_and_regs(to->saved_esp, from);
    scheduler_post_switch();
}

void first_context_switch(void* iret_ptr)
{
    scheduler_post_switch();
    go_to_user_mode(iret_ptr);
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
        .func_addr = first_context_switch,
        .saved_esp = saved_esp,
    };
    PUSH_STACK(tcb->saved_esp, context_stack, context_stack_t);
}
