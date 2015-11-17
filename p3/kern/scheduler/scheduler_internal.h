#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

#define YIELD_MODE 0
#define SCHEDULE_MODE 1

void scheduler_post_switch();
void init_sleep();
void schedule_sleepers(uint32_t current);
void switch_to_next(tcb_t* current, int schedule);
void add_runnable(tcb_t *tcb);
void remove_runnable(tcb_t *tcb, thread_state_t state);

#endif //SCHEDULER_INTERNAL_H
