/** @file scheduler_internal.h
 *
 *  @brief Interface for internal scheduler functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

/** @brief Scheduling modes */
typedef enum {
    YIELD_MODE,
    SCHEDULE_MODE
} schedule_mode_t;

void init_sleep();
void schedule_sleepers(uint32_t current);
void switch_to_next(tcb_t* current, schedule_mode_t schedule);
void add_runnable(tcb_t *tcb);
void remove_runnable(tcb_t *tcb, thread_state_t state);

#endif //SCHEDULER_INTERNAL_H
