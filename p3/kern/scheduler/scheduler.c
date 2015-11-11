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


void scheduler_pre_switch(tcb_t* from, tcb_t* to)
{
    scheduler.switched_from = from;
    switch_ppd(&to->process->directory);
}

void scheduler_post_switch()
{
    tcb_t* switched_from = scheduler.switched_from;
    enable_interrupts();
    if (switched_from->state == EXITED) {
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
    switch_to_next(get_tcb(), SCHEDULE_MODE);
}

/** @brief Schedules the thread to be run
 *
 *  @param tcb Pointer to tcb of thread to schedule
 *  @return void
 */
int schedule(tcb_t* tcb)
{
    // TODO: Neeed mutex to access state
    disable_interrupts();
    if (tcb->state != SUSPENDED && tcb->state != NOT_YET) {
        enable_interrupts();
        return -1;
    }
    tcb->state = RUNNABLE;
    Q_INSERT_TAIL(&scheduler.runnable, tcb, runnable_threads);
    enable_interrupts();
    return 0;
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
    scheduler_mutex_unlock(mp);
    deschedule(tcb);
}


void deschedule(tcb_t *tcb)
{
    tcb->state = SUSPENDED;
    Q_REMOVE(&scheduler.runnable, tcb, runnable_threads);
    switch_to_next(tcb, SCHEDULE_MODE);
}


void kill_thread(tcb_t* tcb, pcb_t *pcb)
{
    disable_interrupts();
    // store the process to free in the tcb
    tcb->process = pcb;
    tcb->state = EXITED;
    Q_REMOVE(&scheduler.runnable, tcb, runnable_threads);
    switch_to_next(tcb, SCHEDULE_MODE);
}


int user_deschedule(tcb_t* tcb, uint32_t esi)
{
    ppd_t* ppd = &tcb->process->directory;
    mutex_lock(&ppd->lock);
    disable_interrupts();
    int reject;
    if (vm_read(ppd, &reject, (void*)esi, sizeof(esi)) < 0) {
        return -1;
    }
    if (reject != 0) {
        return 0;
    }
    scheduler_mutex_unlock(&ppd->lock);
    deschedule(tcb);
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
    tcb_t* yield_tcb = get_tcb_by_id(yield_tid);
    // Thou shalt not yield to threads which don't exist
    if (yield_tcb == NULL) {
        return -1;
    }
    // Thou shalt not yield to the idle thread
    if (yield_tcb->id == scheduler.idle->id) {
        lprintf("Attempted to yield to idle process");
        return -1;
    }
    // Thou shalt not yield to threads which cannot currently be run
    if (yield_tcb->state != RUNNABLE) {
        return -1;
    }
    disable_interrupts();
    context_switch(tcb, yield_tcb);
    return 0;
}
