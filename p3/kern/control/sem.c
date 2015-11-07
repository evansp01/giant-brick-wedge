/** @file sem.c
 *  @brief An implementation of semaphores using mutexes and cond variables
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <sem.h>
#include <mutex.h>
#include <cond.h>
#include <simics.h>

/** @brief Initializes a semaphore for use
 *
 *  Effects of using a semaphore before initialization are undefined.
 *
 *  @param sem Semaphore to initialize
 *  @param count Maximum count for the semaphore
 *  @return 0 for success, -1 for failure
 **/
int sem_init(sem_t* sem, int count)
{
    sem->count = count;
    if (mutex_init(&sem->m) < 0 || cond_init(&sem->cv) < 0) {
        return -1;
    }
    return 0;
}

/** @brief Allows thread to wait for and decrement a semaphore count
 *
 *  Thread will block indefinitely until it is legal to perform the decement.
 *
 *  @param sem Semaphore to wait on
 *  @return Void
 **/
void sem_wait(sem_t* sem)
{
    mutex_lock(&sem->m);
    sem->count--;
    if (sem->count < 0) {
        cond_wait(&sem->cv, &sem->m);
    }
    mutex_unlock(&sem->m);
}

/** @brief Wakes up the next thread waiting on the semaphore
 *
 *  @param sem Semaphore to signal
 *  @return Void
 **/
void sem_signal(sem_t* sem)
{
    mutex_lock(&sem->m);
    sem->count++;
    cond_signal(&sem->cv);
    mutex_unlock(&sem->m);
}

/** @brief Deactivates the semaphore
 *
 *  It is illegal to destroy a semaphore while threads are waiting on it.
 *  Effects of using a semaphore after it has been destroyed are undefined.
 *
 *  @param sem Semaphore to destroy
 *  @return Void
 **/
void sem_destroy(sem_t* sem)
{
    cond_destroy(&sem->cv);
    mutex_destroy(&sem->m);
}
