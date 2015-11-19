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
#include <utilities.h>
#include "scheduler_internal.h"

/** @brief Structure for a list of threads */
Q_NEW_HEAD(runnable_queue_t, tcb);

// Global scheduler list of runnable threads
static struct {
    tcb_t* idle;
    runnable_queue_t runnable;
    tcb_t* switched_from;
    uint32_t ticks;
} scheduler = { 0 };

uint32_t get_ticks()
{
    return scheduler.ticks;
}

void add_runnable(tcb_t* tcb)
{
    tcb->state = T_RUNNABLE;
    Q_INSERT_FRONT(&scheduler.runnable, tcb, runnable_threads);
}

void remove_runnable(tcb_t* tcb, thread_state_t state)
{
    tcb->state = state;
    Q_REMOVE(&scheduler.runnable, tcb, runnable_threads);
}

/** @brief Initializes the scheduler
 *
 *  @return void
 */
void init_scheduler(tcb_t* idle, tcb_t* first)
{
    Q_INIT_HEAD(&scheduler.runnable);
    scheduler.idle = idle;
    add_runnable(first);
    init_sleep();
}

void scheduler_pre_switch(tcb_t* from, tcb_t* to)
{
    scheduler.switched_from = from;
    switch_ppd(to->process->directory);
}

void scheduler_post_switch()
{
    tcb_t* switched_from = scheduler.switched_from;
    thread_state_t state = switched_from->state;
    enable_interrupts();
    if (state == T_EXITED) {
        finalize_exit(switched_from);
    }
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
 *  @return void
 */
void schedule_interrupts_disabled(tcb_t* tcb, thread_state_t expected)
{
    if (tcb->state != expected) {
        panic("Things are not as expected");
    }
    add_runnable(tcb);
}

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

/** @brief Deschedules the current thread
 *
 *  Runs the next thread without re-scheduling the current thread
 *
 *  @return void
 */
void deschedule_and_drop(tcb_t* tcb, mutex_t* mp, thread_state_t new_state)
{
    disable_interrupts();
    scheduler_mutex_unlock(mp);
    remove_runnable(tcb, new_state);
    switch_to_next(tcb, YIELD_MODE);
}

void deschedule(tcb_t* tcb, thread_state_t new_state)
{
    disable_interrupts();
    remove_runnable(tcb, new_state);
    switch_to_next(tcb, YIELD_MODE);
}

void kill_thread(tcb_t* tcb, ppd_t* ppd)
{
    disable_interrupts();
    // store the process to free in the tcb
    tcb->free_pointer = ppd;
    remove_runnable(tcb, T_EXITED);
    switch_to_next(tcb, YIELD_MODE);
}

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
    if (yield_tcb == NULL || yield_tcb->state != T_RUNNABLE) {
        mutex_unlock(&kernel_state.threads_mutex);
        return -1;
    }
    disable_interrupts();
    scheduler_mutex_unlock(&kernel_state.threads_mutex);
    context_switch(tcb, yield_tcb);
    return 0;
}
