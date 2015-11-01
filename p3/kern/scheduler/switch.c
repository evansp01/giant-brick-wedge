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
    lprintf("Switching from %d to %d", from->id, to->id);
    switch_ppd(&to->parent->directory);
    switch_context(to->saved_esp, from);
    lprintf("Now running %d", from->id);
}

/** @brief Yields execution to another thread
 *
 *  @param yield_tid Thread id of thread to yield to
 *  @return Zero on success, an integer less than zero on failure
 **/
int yield(int yield_tid)
{
    // Get scheduler to choose next thread to run if tid is -1
    if (yield_tid == -1) {
        run_next();
        return 0;
    }

    tcb_t* p_tcb = get_tcb();

    // User has requested to yield to the currently running thread
    if (yield_tid == p_tcb->id) {
        return 0;
    }

    extern kernel_state_t kernel_state;
    tcb_t* tcb;
    Q_FOREACH(tcb, &kernel_state.threads, all_threads) {
        if (tcb->id == yield_tid) {
            disable_interrupts();
            lprintf("Yield -- Switching from %d to %d", p_tcb->id, tcb->id);
            switch_context_ppd(p_tcb, tcb);
            enable_interrupts(); // TODO: Switch to scheduler mutex?
            return 0;
        }
    }
    return -1;
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
