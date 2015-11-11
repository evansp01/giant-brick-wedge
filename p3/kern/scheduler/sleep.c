#include <variable_queue.h>
#include <variable_htable.h>
#include <control.h>
#include <scheduler.h>
#include <asm.h>
#include <simics.h>

H_NEW_TABLE(sleep_hash_t, tcb_ds_t);

static sleep_hash_t table;

int init_sleep()
{
    return H_INIT_TABLE(&table);
}

uint32_t add_sleeper(tcb_t* tcb, uint32_t ticks)
{
    if (ticks < 0) {
        return -1;
    }
    if (ticks == 0) {
        return 0;
    }
    // get the malloc lock
    acquire_malloc();
    // disable interrupts to prevent switch
    disable_interrupts();
    // release the malloc lock -- we that malloc is unlocked until we 
    // enable interrupts again (unless we lock it)
    // TODO: just hold malloc the whole time (a bit of a pain though)
    // TODO: reentrant locks?? -- would really make this less of a pain
    release_malloc();

    uint32_t until = get_ticks() + ticks;
    lprintf("%lu until", until);
    while (H_CONTAINS(&table, until, wake_tick, suspended_threads)) {
        until++;
    }
    tcb->wake_tick = until;
    H_INSERT(&table, tcb, wake_tick, suspended_threads);
    deschedule(tcb);
    enable_interrupts();
    return until;
}

void schedule_sleepers(uint32_t current)
{
    // can't do anything which might malloc since we are in the timer int
    disable_interrupts();
    tcb_t* tcb = H_GET(&table, current, wake_tick, suspended_threads);
    enable_interrupts();
    schedule(tcb);
}


void release_sleeper(uint32_t slept){
    acquire_malloc();
    // disable interrupts to prevent switch
    disable_interrupts();
    H_REMOVE(&table, slept, wake_tick, suspended_threads);
    enable_interrupts();

}
