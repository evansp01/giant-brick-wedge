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

void switch_context_ppd(tcb_t* from, tcb_t* to)
{
    switch_ppd(&to->parent->directory);
    switch_context(to->saved_esp, from);
}


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
