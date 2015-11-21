/** @file rwlock.c
 *  @brief An implementation of reader/writer locks
 *
 *  This implementation attempts to prevent starvation of both readers and
 *  writers. Queues for both readers and writers are maintained using condition
 *  variables.
 *
 *  Whenever one reader acquires the lock, all readers waiting in line will
 *  also acquire the lock. If there are writers waiting, new readers will have
 *  to wait for those writers to finish before proceeding. Whenever a writer
 *  joins the waiting queue and the lock is in read mode, all future readers
 *  will have to wait.
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <rwlock.h>
#include <mutex.h>
#include <cond.h>
#include <thread.h>
#include <simics.h>

/** @brief Initializes a reader/writer lock for use
 *
 *  @param rwlock Reader/writer lock to initialize
 *  @return 0 for success, -1 for failure
 **/
int rwlock_init(rwlock_t *rwlock)
{
    rwlock->mode = RWLOCK_READ;
    rwlock->writers_waiting = 0;
    rwlock->writers_queued = 0;
    rwlock->readers_waiting = 0;
    rwlock->readers_active = 0;
    rwlock->owner = -1;
    if (mutex_init(&rwlock->m) < 0 || cond_init(&rwlock->cv_readers) < 0
        || cond_init(&rwlock->cv_writers) < 0) {
        return -1;
    }
    return 0;
}

/** @brief Destroys a reader/writer lock after use
 *
 *  @param rwlock Reader/writer lock to destroy
 *  @return Void
 **/
void rwlock_destroy(rwlock_t *rwlock)
{
    mutex_destroy(&rwlock->m);
    cond_destroy(&rwlock->cv_readers);
    cond_destroy(&rwlock->cv_writers);
}

/** @brief Locks a reader/writer lock for the given mode type
 *
 *  @param rwlock Reader/writer lock to be locked
 *  @param type Type of lock
 *  @return Void
 **/
void rwlock_lock (rwlock_t *rwlock, int type)
{
    mutex_lock(&rwlock->m);

    // A reader is attempting to get the lock
    if (type == RWLOCK_READ) {

        // If there are writers queued then wait for next reader broadcast
        if (rwlock->writers_waiting > 0 || rwlock->writers_queued > 0) {
            rwlock->readers_waiting++;
            cond_wait(&rwlock->cv_readers, &rwlock->m);
            rwlock->readers_waiting--;
        }

        // Carry on with reading
        rwlock->readers_active++;
    }

    // A writer is attempting to get the lock
    else if (type == RWLOCK_WRITE) {

        // If there is currently anyone active then wait in the queue
        if (rwlock->readers_active > 0 || rwlock->writers_queued > 0 ||
            rwlock->readers_waiting > 0) {

            // No readers are waiting, so place writer in active queue
            if (rwlock->readers_waiting == 0)
                rwlock->writers_queued++;
            else
                rwlock->writers_waiting++;

            cond_wait(&rwlock->cv_writers, &rwlock->m);
        }

        // Otherwise carry on writing immediately
        else {
            rwlock->writers_queued++;
        }

        // Set writer as owner of the lock
        rwlock->owner = thr_getid();
    }

    // Now in possession of the rwlock, so set mode to type
    rwlock->mode = type;
    mutex_unlock(&rwlock->m);
}

/** @brief Unlocks the reader/writer lock that the current thread holds
 *
 *  Behavior is undefined if the calling thread is not a current owner of
 *  the reader/writer lock
 *
 *  @param rwlock Reader/writer lock to be unlocked
 *  @return Void
 **/
void rwlock_unlock(rwlock_t *rwlock)
{
    mutex_lock(&rwlock->m);

    /* Lock is currently in READ mode
     * State: At least one reader is currently active
     */
    if (rwlock->mode == RWLOCK_READ) {
        rwlock->readers_active--;

        // If all the current readers are done
        if (rwlock->readers_active == 0) {

            /* Signal the next waiting writers, if any.
             * Note that there are no waiting readers if there are no waiting
             * writers.
             * There can be queued writers due to rwlock_downgrade().
             */
            if (rwlock->writers_queued > 0) {
                cond_signal(&rwlock->cv_writers);
            }
            else if (rwlock->writers_waiting > 0) {
                rwlock->writers_queued = rwlock->writers_waiting;
                rwlock->writers_waiting = 0;
                cond_signal(&rwlock->cv_writers);
            }

        }
    }

    /* Lock is currently in WRITE mode
     * State: A writer is currently active
     */
    else if (rwlock->mode == RWLOCK_WRITE && rwlock->owner == thr_getid()) {
        rwlock->writers_queued--;

        // If there are still queued writers
        if (rwlock->writers_queued > 0) {
            cond_signal(&rwlock->cv_writers);
        }

        // If all the current writers queued are done
        else {

            // If there are readers waiting, broadcast to let them go
            if (rwlock->readers_waiting > 0) {
                cond_broadcast(&rwlock->cv_readers);
            }

            // If there are no readers waiting, queue the next batch of writers
            else if (rwlock->writers_waiting > 0) {
                rwlock->writers_queued = rwlock->writers_waiting;
                rwlock->writers_waiting = 0;
                cond_signal(&rwlock->cv_writers);
            }
        }
    }

    mutex_unlock(&rwlock->m);
}

/** @brief Downgrades the reader/writer lock that the current thread holds
 *
 *  Does nothing if the lock is not in WRITE mode, or if the current thread
 *  if not the owner of the lock
 *
 *  @param rwlock Reader/writer lock to be downgraded
 *  @return Void
 **/
void rwlock_downgrade(rwlock_t *rwlock)
{
    mutex_lock(&rwlock->m);

    // If the calling thread already owns the lock in RWLOCK_WRITE mode
    if (rwlock->mode == RWLOCK_WRITE && rwlock->owner == thr_getid()) {
        rwlock->writers_queued--;
        rwlock->mode = RWLOCK_READ;
        rwlock->readers_active++;
        cond_broadcast(&rwlock->cv_readers);
    }

    mutex_unlock(&rwlock->m);
}
