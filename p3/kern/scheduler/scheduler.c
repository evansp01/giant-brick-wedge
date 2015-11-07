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

#define YIELD_MODE 0
#define SCHEDULE_MODE 1

/** @brief Structure for a list of threads */
Q_NEW_HEAD(runnable_queue_t, tcb);

// Global scheduler list of runnable threads
struct {
    tcb_t* idle;
    runnable_queue_t runnable;
} scheduler = { 0 };

/** @brief Initializes the scheduler
 *
 *  @return void
 */
void init_scheduler(tcb_t* idle, tcb_t* first)
{
    Q_INIT_HEAD(&scheduler.runnable);
    scheduler.idle = idle;
    Q_INSERT_TAIL(&scheduler.runnable, first, runnable_threads);
}

/** @brief Switches to the next thread to be run
 *
 *  @param current The tcb of the current thread
 *  @param schedule Should this switch update the scheduling queue
 *  @return void
 **/
void switch_to_next(tcb_t* current, int schedule)
{
    if (!Q_IS_EMPTY(&scheduler.runnable)) {
        tcb_t* next = Q_GET_FRONT(&scheduler.runnable);
        if (schedule) {
            Q_REMOVE(&scheduler.runnable, next, runnable_threads);
            Q_INSERT_TAIL(&scheduler.runnable, next, runnable_threads);
        }
        if (current->id != next->id) {
            switch_context_ppd(current, next);
        }
    } else {
        // no runnable threads, should run idle
        switch_context_ppd(current, scheduler.idle);
    }
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
    tcb_t* tcb = get_tcb();
    switch_to_next(tcb, SCHEDULE_MODE);
    enable_interrupts();
}

/** @brief Schedules the thread to be run
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @return void
 */
void schedule(tcb_t* tcb)
{
    // TODO: Neeed mutex to access state
    tcb->state = RUNNABLE;
    disable_interrupts();
    Q_INSERT_TAIL(&scheduler.runnable, tcb, runnable_threads);
    enable_interrupts();
}

/** @brief Deschedules the current thread
 *  
 *  Runs the next thread without re-scheduling the current thread
 *
 *  @return void
 */
void deschedule_and_drop(tcb_t* tcb, mutex_t* mp)
{
    disable_interrupts();
    mutex_unlock(mp);
    Q_REMOVE(&scheduler.runnable, tcb, runnable_threads);
    switch_to_next(tcb, SCHEDULE_MODE);
    enable_interrupts();
}

void deschedule(tcb_t* tcb)
{
    disable_interrupts();
    Q_REMOVE(&scheduler.runnable, tcb, runnable_threads);
    switch_to_next(tcb, SCHEDULE_MODE);
    enable_interrupts();
}

/** @brief Yields execution to another thread
 *
 *  @param yield_tid Thread id of thread to yield to
 *  @return Zero on success, an integer less than zero on failure
 **/
int yield(int yield_tid)
{
    tcb_t* tcb = get_tcb();
    // User has requested to yield to the currently running thread
    if (yield_tid == tcb->id) {
        return 0;
    }
    // Get scheduler to choose next thread to run if tid is -1
    if (yield_tid == -1) {
        disable_interrupts();
        switch_to_next(tcb, YIELD_MODE);
        enable_interrupts();
        return 0;
    }
    // Yield to a specific thread
    tcb_t* yield_tcb = get_tcb_by_id(yield_tid);
    if (yield_tcb) {
        disable_interrupts();
        switch_context_ppd(tcb, yield_tcb);
        enable_interrupts();
        return 0;
    }
    return -1;
}
