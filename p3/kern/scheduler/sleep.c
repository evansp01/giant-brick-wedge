#include <variable_queue.h>
#include <variable_htable.h>
#include <control.h>
#include <scheduler.h>
#include <asm.h>
#include <simics.h>

static tcb_ds_t sleep_list;
static mutex_t sleep_mutex;

void init_sleep()
{
    Q_INIT_HEAD(&sleep_list);
    mutex_init(&sleep_mutex);
}

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
    if(until < iter->wake_tick){
        Q_INSERT_BEFORE(&sleep_list, iter, tcb, sleeping_threads);
    } else {
        Q_INSERT_AFTER(&sleep_list, iter, tcb, sleeping_threads);
    }
    remove_runnable(tcb, SLEEPING);
    enable_interrupts();
    mutex_unlock(&sleep_mutex);
    return 1;
}

void schedule_sleepers(uint32_t current)
{
    disable_interrupts();
    // if we only examine the list, we don't need the lock
    tcb_t *head = Q_GET_FRONT(&sleep_list);
    if(head->wake_tick <= current && head->state == SLEEPING){
        add_runnable(head);
    }
    enable_interrupts();
}

void release_sleeper(tcb_t *sleeper)
{
    mutex_lock(&sleep_mutex);
    disable_interrupts();
    Q_REMOVE(&sleep_list, sleeper, sleeping_threads);
    enable_interrupts();
    mutex_unlock(&sleep_mutex);
}
