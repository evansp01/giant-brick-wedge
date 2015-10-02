#include <sem.h>
#include <sem_type.h>
#include <mutex.h>
#include <cond.h>

int sem_init(sem_t* sem, int count)
{
    sem->count = count;
    mutex_init(&sem->m);
    cond_init(&sem->cv);
    return 0;
}

void sem_wait(sem_t* sem)
{
}

void sem_signal(sem_t* sem)
{
}

void sem_destroy(sem_t* sem)
{
}
