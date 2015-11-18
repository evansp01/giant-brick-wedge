/** @file cond.c
 *  @brief An implementation of condition variables
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <cond.h>
#include <simics.h>
#include <stdlib.h>
#include <control.h>
#include <asm.h>
#include <scheduler.h>

/** @brief Initialize a condition variable allocating needed resources
 *  It is not valid to wait, signal, or broadcast on a condition variable
 *  before it has been initialized
 *
 *  @param cv The condition variable to initialize
 *  @return void
 */
void cond_init(cond_t* cv)
{
    Q_INIT_HEAD(&cv->waiting);
}

/** @brief Destroy a condition variable freeing all associated resources
 *  It is illegal to destroy a condition variable while there are threads
 *  waiting on it
 *
 *  @param cv The condition variable to destroy
 *  @return void
 **/
void cond_destroy(cond_t* cv)
{
    if (!Q_IS_EMPTY(&cv->waiting)) {
        panic("cond var destroyed while threads are waiting");
    }
}

/** @brief Wait on a condition variable until signaled by cond_signal
 *
 *  @param cv The condition variable to wait on
 *  @param mp The mutex to wait with
 *  @return void
 **/
void cond_wait(cond_t* cv, mutex_t* mp)
{
    tcb_t *tcb = get_tcb();
    disable_interrupts();
    Q_INSERT_TAIL(&cv->waiting, tcb, suspended_threads);
    scheduler_mutex_unlock(mp);
    deschedule(tcb);
    mutex_lock(mp);
}

/** @brief Signal a waiting thread if such a thread exists
 *
 *  @param cv The condition variable to signal on
 *  @return void
 **/
void cond_signal(cond_t* cv)
{
    disable_interrupts();
    if (!Q_IS_EMPTY(&cv->waiting)) {
        tcb_t *tcb_to_schedule = Q_GET_FRONT(&cv->waiting);
        Q_REMOVE(&cv->waiting, tcb_to_schedule, suspended_threads);
        schedule(tcb_to_schedule);
    }
    enable_interrupts();
}

/** @brief Signal all threads currently waiting on the condition variable
 *
 *  @param cv The condition variable to broadcast on
 *  @return void
 **/
void cond_broadcast(cond_t* cv)
{
    disable_interrupts();
    while (!Q_IS_EMPTY(&cv->waiting)) {
        tcb_t *tcb_to_schedule = Q_GET_FRONT(&cv->waiting);
        Q_REMOVE(&cv->waiting, tcb_to_schedule, suspended_threads);
        schedule(tcb_to_schedule);
    }
    enable_interrupts();
}
