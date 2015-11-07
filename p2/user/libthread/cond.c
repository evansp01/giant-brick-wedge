/** @file cond.c
 *  @brief An implementation of condition variables
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <cond.h>
#include <thr_internals.h>
#include <syscall.h>
#include <thread.h>
#include <simics.h>
#include <errors.h>

/** @brief Initialize a condition variable allocating needed resources
 *  It is not valid to wait, signal, or broadcast on a condition variable
 *  before it has been initialized
 *
 *  @param cv The condition variable to initialize
 *  @return zero on success, less than zero on failure
 */
int cond_init(cond_t* cv)
{
    if (mutex_init(&cv->m) < 0) {
        return -1;
    }
    Q_INIT_HEAD(&cv->waiting);
    return 0;
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
    mutex_destroy(&cv->m);
    if (Q_GET_FRONT(&cv->waiting) != NULL) {
        EXIT_ERROR("condition variable destroyed, but queue not empty");
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
    node_t node;
    node.tid = thr_getid();
    node.reject = 0;
    Q_INIT_ELEM(&node, node_link);
    mutex_lock(&cv->m);
    Q_INSERT_TAIL(&cv->waiting, &node, node_link);
    mutex_unlock(mp);
    mutex_unlock(&cv->m);
    deschedule(&node.reject);
    mutex_lock(mp);
}

/** @brief Signal a waiting thread if such a thread exists
 *
 *  @param cv The condition variable to signal on
 *  @return void
 **/
void cond_signal(cond_t* cv)
{
    mutex_lock(&cv->m);
    // if nobody is waiting, just return
    if (Q_GET_FRONT(&cv->waiting) != NULL) {
        node_t *node = Q_GET_FRONT(&cv->waiting);
        Q_REMOVE(&cv->waiting, node, node_link);
        node->reject = make_runnable(node->tid);
    }
    mutex_unlock(&cv->m);
}

/** @brief Signal all threads currently waiting on the condition variable
 *
 *  @param cv The condition variable to broadcast on
 *  @return void
 **/
void cond_broadcast(cond_t* cv)
{
    mutex_lock(&cv->m);
    // if nobody is waiting, just return
    while (Q_GET_FRONT(&cv->waiting) != NULL) {
        node_t *node = Q_GET_FRONT(&cv->waiting);
        Q_REMOVE(&cv->waiting, node, node_link);
        node->reject = make_runnable(node->tid);
    }
    mutex_unlock(&cv->m);
}
