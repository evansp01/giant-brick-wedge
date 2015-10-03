#include <sem.h>
#include <sem_type.h>
#include <mutex.h>
#include <cond.h>
#include <simics.h>

int sem_init(sem_t* sem, int count)
{
    sem->count = count;
    if (mutex_init(&sem->m) < 0 || cond_init(&sem->cv) < 0) {
        return -1;
    }
    return 0;
}

void sem_wait(sem_t* sem)
{
    mutex_lock(&sem->m);
    sem->count--;
    if (sem->count < 0) {
        cond_wait(&sem->cv, &sem->m);
    }
    mutex_unlock(&sem->m);
}

void sem_signal(sem_t* sem)
{
    mutex_lock(&sem->m);
    sem->count++;
    cond_signal(&sem->cv);
    mutex_unlock(&sem->m);
}

void sem_destroy(sem_t* sem)
{
    cond_destroy(&sem->cv);
    mutex_destroy(&sem->m);
}
