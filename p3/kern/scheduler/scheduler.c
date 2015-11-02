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
tcb_t *scheduled_tcb;
 
/** @brief Initializes the scheduler
 *
 *  @return void
 */
void init_scheduler()
{
    Q_INIT_HEAD(&runnable);
    scheduled_tcb = NULL;
}
 
/** @brief Runs the next thread in the runnable queue
 *  
 *  Interrupts need to be disabled since run_next() can be called in yield()
 *  and not just from within the scheduler.
 *
 *  @return void
 */
void run_next()
{
    disable_interrupts();
    if (!Q_IS_EMPTY(&runnable)) {
        tcb_t *curr_tcb = get_tcb();
        if(scheduled_tcb == NULL){
            schedule(curr_tcb);
        } else {
            schedule(scheduled_tcb);
        }
        tcb_t *next_tcb = Q_GET_FRONT(&runnable);
        scheduled_tcb = next_tcb;
        Q_REMOVE(&runnable, next_tcb, runnable_threads);
        lprintf("Timer -- Switching from %d to %d", curr_tcb->id, next_tcb->id);
        if(curr_tcb->id != next_tcb->id){
            switch_context_ppd(curr_tcb, next_tcb);
        }
    }
    
    enable_interrupts();
}

/** @brief Schedules the thread to be run
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @return void
 */
void schedule(tcb_t *tcb)
{
    // TODO: Need to disable interrupts when adding to queue?
    //       Or maybe switch to using a mutex to lock?    
    disable_interrupts();
    tcb->state = RUNNABLE;
    Q_INSERT_TAIL(&runnable, tcb, runnable_threads);
    enable_interrupts();
}

/** @brief Re-schedule the thread if it was scheduled at the previous timer tick
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @return void
 */
void cond_schedule(tcb_t *tcb)
{
    // Re-schedule only if yield did not occur between timer ticks
    if (tcb == scheduled_tcb) {
        schedule(tcb);
    }
}

