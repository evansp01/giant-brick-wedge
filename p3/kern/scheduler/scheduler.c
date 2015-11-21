/** @file scheduler.c
 *
 *  @brief Functions to schedule threads
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control_block.h>
#include <scheduler.h>
#include <switch.h>
#include <variable_queue.h>
#include <simics.h>
#include <asm.h>
#include <contracts.h>
#include <malloc_wrappers.h>
#include "scheduler_internal.h"

/** @brief Structure for a list of threads */
Q_NEW_HEAD(runnable_queue_t, tcb);

/** @brief The ratio of p0 threads to p1 threads run */
#define P0_PRIORITY 2

/** @brief State of the scheduler including queues and number of ticks */
static struct {
    tcb_t* idle;
    runnable_queue_t runnable_p0;
    runnable_queue_t runnable_p1;
    int p0_run_count;
    uint32_t ticks;
} scheduler = { 0 };

/** @brief Gets the number of ticks since system start
 *
 *  @return The number of ticks
 **/
uint32_t get_ticks()
{
    return scheduler.ticks;
}

/** @brief Add a thread to the runnable list
 *
 *  @param tcb The thread to add
 *  @return void
 **/
void add_runnable(tcb_t* tcb)
{
    tcb->state = T_RUNNABLE_P0;
    // add threads to the end of the high priority queue
    Q_INSERT_TAIL(&scheduler.runnable_p0, tcb, runnable_threads);
}

/** @brief Is this thread runnable
 *
 *  @param tcb The thread to check
 *  @return A boolean integer
 **/
int is_runnable(tcb_t* tcb)
{
    return tcb->state == T_RUNNABLE_P0 || tcb->state == T_RUNNABLE_P1;
}

/** @brief Remove a thread from the runnable list, and update it's state
 *
 *  @param tcb The thread to remove
 *  @param state The new state of the removed thread
 **/
void remove_runnable(tcb_t* tcb, thread_state_t state)
{
    if (tcb->state == T_RUNNABLE_P0) {
        Q_REMOVE(&scheduler.runnable_p0, tcb, runnable_threads);
    } else if (tcb->state == T_RUNNABLE_P1) {
        Q_REMOVE(&scheduler.runnable_p1, tcb, runnable_threads);
    } else {
        panic("Removing runnable called on thread which is not runnable");
    }
    tcb->state = state;
}

/** @brief Get the next runnable thread
 *
 *  @return The next runnable thread or NULL if there is no runnable thread
 **/
tcb_t* get_next_runnable()
{
    // if there is a low priority thread to run and we have run too many
    // high priority threads recently
    if (scheduler.p0_run_count >= P0_PRIORITY &&
            !Q_IS_EMPTY(&scheduler.runnable_p1)) {
        scheduler.p0_run_count = 0;
        return Q_GET_FRONT(&scheduler.runnable_p1);
    }
    // if there is a high priority thread to run
    if (!Q_IS_EMPTY(&scheduler.runnable_p0)) {
        scheduler.p0_run_count++;
        return Q_GET_FRONT(&scheduler.runnable_p0);
    }
    // if there is a low priority thread to run
    if (!Q_IS_EMPTY(&scheduler.runnable_p1)) {
        scheduler.p0_run_count = 0;
        return Q_GET_FRONT(&scheduler.runnable_p1);
    }
    // if there is no thread to run
    return NULL;
}

/** @brief Rotate the runnable queue
 *
 *  Moves the thread at the front to the end of the p1 runnable queue
 *
 *  @return void
 **/
void rotate_runnable()
{
    tcb_t* next = get_next_runnable();
    // add the thread to the low priority queue
    remove_runnable(next, T_RUNNABLE_P1);
    Q_INSERT_TAIL(&scheduler.runnable_p1, next, runnable_threads);
}

/** @brief Initializes the scheduler
 *
 *  @param idle The idle thread
 *  @param first The first thread which should be run
 *
 *  @return void
 */
void init_scheduler(tcb_t* idle, tcb_t* first)
{
    Q_INIT_HEAD(&scheduler.runnable_p0);
    Q_INIT_HEAD(&scheduler.runnable_p1);
    scheduler.idle = idle;
    add_runnable(first);
    init_sleep();
}

/** @brief Switches to the next thread to be run
 *
 *  @param current The tcb of the current thread
 *  @param schedule Should this switch update the scheduling queue
 *  @return void
 **/
void switch_to_next(tcb_t* current, schedule_mode_t schedule)
{

    tcb_t* next = get_next_runnable();
    if (next != NULL) {
        if (schedule) {
            rotate_runnable();
        }

        if (current->id != next->id) {
            context_switch(current, next);
        }
    } else {
        // no runnable threads, should run idle
        if (current->id != scheduler.idle->id) {
            context_switch(current, scheduler.idle);
        }
    }
    // must re-enable interrupts if not context switching
    enable_interrupts();
}

/** @brief Runs the next thread in the runnable queue
 *
 *  Interrupts need to be disabled since run_next() can be called in yield()
 *  and not just from within the scheduler.
 *
 *  @param ticks The number of ticks since system boot
 *
 *  @return void
 */
void run_scheduler(uint32_t ticks)
{
    disable_interrupts();
    scheduler.ticks = ticks;
    schedule_sleepers(ticks);
    switch_to_next(get_tcb(), SCHEDULE_MODE);
}

/** @brief Schedules the thread to be run
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @param expected The state the scheduling thread expects to find the tcb in
 *  @return void
 */
void schedule(tcb_t* tcb, thread_state_t expected)
{
    disable_interrupts();
    schedule_interrupts_disabled(tcb, expected);
    enable_interrupts();
}

/** @brief Schedules the thread to be run
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @param expected The state the scheduling thread expects to find the tcb in
 *  @return void
 */
void schedule_interrupts_disabled(tcb_t* tcb, thread_state_t expected)
{
    if (tcb->state != expected) {
        panic("Thread schedule attempted, thread not in expected state");
    }
    add_runnable(tcb);
}

/** @brief Schedule a thread using the make_runnable thread
 *
 *  The thread must have been suspended by the user
 *
 *  @param tcb The thread the schedule
 *  @param mp The thread list mutex, which we will drop
 *  @return Zero on success, less than zero on failure
 **/
int user_schedule(tcb_t* tcb, mutex_t* mp)
{
    disable_interrupts();
    scheduler_mutex_unlock(mp);
    if (tcb->state != T_SUSPENDED) {
        enable_interrupts();
        return -1;
    }
    add_runnable(tcb);
    enable_interrupts();
    return 0;
}

/** @brief Deschedules the current thread, and drops a mutex
 *
 *  @param tcb The thread to deschedule
 *  @param mp The mutex to drop
 *  @param new_state The threads new state upon being descheduled
 *  @return void
 **/
void deschedule_and_drop(tcb_t* tcb, mutex_t* mp, thread_state_t new_state)
{
    disable_interrupts();
    scheduler_mutex_unlock(mp);
    remove_runnable(tcb, new_state);
    switch_to_next(tcb, YIELD_MODE);
}

/** @brief Deschedules the current thread
 *
 *  @param tcb The thread to deschedule
 *  @param new_state The threads new state upon being descheduled
 *  @return void
 **/
void deschedule(tcb_t* tcb, thread_state_t new_state)
{
    disable_interrupts();
    remove_runnable(tcb, new_state);
    switch_to_next(tcb, YIELD_MODE);
}

/** @brief Kills the current thread, setting it's exit status to T_EXITED
 *
 *  kill_thread must be called with the malloc_mutex held, so that the threads
 *  memory cannot be freed before it is deschedule, kill_thread will drop
 *  this mutex
 *
 *  @param tcb The thread to kill
 *  @return void
 **/
void kill_thread(tcb_t* tcb)
{
    disable_interrupts();
    scheduler_release_malloc();
    remove_runnable(tcb, T_EXITED);
    switch_to_next(tcb, YIELD_MODE);
}

/** @brief Deschedule a runnable thread using the deschedule call
 *
 *  @param tcb The thread to deschedule
 *  @param esi The reject pointer passed by the user
 *  @return Zero on success, less than zero on failure
 **/
int user_deschedule(tcb_t* tcb, uint32_t esi)
{
    ppd_t* ppd = tcb->process->directory;
    mutex_lock(&ppd->lock);
    disable_interrupts();
    int reject;
    if (vm_read(ppd, &reject, (void*)esi, sizeof(esi)) < 0) {
        mutex_unlock(&ppd->lock);
        return -1;
    }
    if (reject != 0) {
        mutex_unlock(&ppd->lock);
        return 0;
    }
    scheduler_mutex_unlock(&ppd->lock);
    remove_runnable(tcb, T_SUSPENDED);
    switch_to_next(tcb, SCHEDULE_MODE);
    return 0;
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
        return 0;
    }
    // Yield to a specific thread
    mutex_lock(&kernel_state.threads_mutex);
    tcb_t* yield_tcb = get_tcb_by_id(yield_tid);
    // Thou shalt not yield to threads which don't exist, or are not runnable
    disable_interrupts();
    if (yield_tcb == NULL || !is_runnable(yield_tcb)) {
        enable_interrupts();
        mutex_unlock(&kernel_state.threads_mutex);
        return -1;
    }
    scheduler_mutex_unlock(&kernel_state.threads_mutex);
    context_switch(tcb, yield_tcb);
    return 0;
}
