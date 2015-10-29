/** @file scheduler.c
 *
 *  @brief Functions to schedule threads
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control.h>
#include <scheduler.h>
#include <switch.h>
#include <variable_queue.h>
#include <simics.h>
#include <asm.h>
 
// Global scheduler list of runnable threads
runnable_queue_t runnable;
 
/** @brief Initializes the scheduler
 *
 *  @return void
 */
void init_scheduler()
{
    Q_INIT_HEAD(&runnable);
}
 
/** @brief Runs the next thread in the runnable queue
 *
 *  @return void
 */
void run_next()
{
    //disable_interrupts();
    
    if (!Q_IS_EMPTY(&runnable)) {
        tcb_t *curr_tcb = get_tcb();
        schedule(curr_tcb);
        tcb_t *next_tcb = Q_GET_FRONT(&runnable);
        Q_REMOVE(&runnable, next_tcb, runnable_threads);
        switch_context(next_tcb->saved_esp, curr_tcb);
    }
    
    //enable_interrupts();
}

/** @brief Schedules the thread to be run
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @return void
 */
void schedule(tcb_t *tcb)
{
    tcb->state = RUNNABLE;
    Q_INSERT_TAIL(&runnable, tcb, runnable_threads);
}


