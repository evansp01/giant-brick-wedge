/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */



#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

void frame_alloc_init(unsigned int size, void *stack_high, void *stack_low);
void* alloc_frame();
void free_frame(void* frame);

#endif /* THR_INTERNALS_H */
