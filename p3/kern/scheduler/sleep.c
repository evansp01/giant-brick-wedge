/** @file sleep.c
 *
 *  @brief Functions to sleep threads
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <variable_queue.h>
#include <control_block.h>
#include <scheduler.h>
#include <asm.h>
#include <simics.h>
#include "scheduler_internal.h"

Q_NEW_HEAD(sleeper_list_t, tcb);
static sleeper_list_t sleep_list;
static mutex_t sleep_mutex;


/** @brief Initialize state needed for the sleep system call
 *  @return void
 **/
void init_sleep()
{
    Q_INIT_HEAD(&sleep_list);
    mutex_init(&sleep_mutex);
}

/** @brief Add a the current thread to the list of sleeping threads
 *  @param tcb The tcb of the current thread;w
 *  @param ticks The number of ticks to sleep for
 **/
int add_sleeper(tcb_t* tcb, uint32_t ticks)
{
    if (ticks < 0) {
        return -1;
    }
    if (ticks == 0) {
        return 0;
    }
    mutex_lock(&sleep_mutex);
    tcb_t* iter;
    uint32_t until = get_ticks() + ticks;
    // O(n) time holding the lock
    Q_FOREACH(iter, &sleep_list, sleeping_threads)
    {
        if (iter->wake_tick >= until) {
            break;
        }
    }
    // O(1) disable interrupts time
    disable_interrupts();
    // if iter is the last element we might want to insert after
    tcb->wake_tick = until;
    if(until < iter->wake_tick){
        Q_INSERT_BEFORE(&sleep_list, iter, tcb, sleeping_threads);
    } else {
        Q_INSERT_AFTER(&sleep_list, iter, tcb, sleeping_threads);
    }
    deschedule_and_drop(tcb, &sleep_mutex, T_SLEEPING);
    return 1;
}

/** @brief Schedule the sleeping thread at the start of the sleeping list
 *         if it is time for it to wake up
 *
 *  Note: Should be called from the scheduler with interrupts disabled
 *
 *  @param current The current number of ticks
 *  @return void
 **/
void schedule_sleepers(uint32_t current)
{
    // if we only examine the list, we don't need the lock
    tcb_t *head = Q_GET_FRONT(&sleep_list);
    if(head == NULL){
        return;
    }
    if(head->wake_tick <= current && head->state == T_SLEEPING){
        schedule_interrupts_disabled(head, T_SLEEPING);
    }
}

/** @brief After a sleeping thread has awoken it must remove itsself from the
 *         sleeping list with this call
 *
 *  @param sleeper The thread which woke up
 *  @return void
 **/
void release_sleeper(tcb_t *sleeper)
{
    mutex_lock(&sleep_mutex);
    disable_interrupts();
    Q_REMOVE(&sleep_list, sleeper, sleeping_threads);
    enable_interrupts();
    mutex_unlock(&sleep_mutex);
}
