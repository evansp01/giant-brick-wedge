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
#include <contracts.h>
 
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

/** @brief Deschedules the current thread
 *  
 *  Runs the next thread without re-scheduling the current thread
 *
 *  @return void
 */
void deschedule(tcb_t *tcb)
{
    // interrupts are disabled before call to deschedule
    if (!Q_IS_EMPTY(&runnable)) {
        tcb_t *curr_tcb = get_tcb();
        if ((scheduled_tcb != NULL)&&(scheduled_tcb != curr_tcb)) {
            schedule(scheduled_tcb);
        }
        // Remove thread from runnable list (if it happens to be queued)
        Q_REMOVE(&runnable, curr_tcb, runnable_threads);
        
        tcb_t *next_tcb = Q_GET_FRONT(&runnable);
        Q_REMOVE(&runnable, next_tcb, runnable_threads);
        scheduled_tcb = next_tcb;
        ASSERT(curr_tcb->id != next_tcb->id);
        switch_context_ppd(curr_tcb, next_tcb);
    }
    // Descheduling the last thread, switch to idle thread
    else {
        // TODO: Switch to idle thread
    }
    enable_interrupts();
}
