/** @file malloc_wrappers.h
 *  @brief Interface for malloc wrapper functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_MALLOC_WRAPPERS_H
#define KERN_INC_MALLOC_WRAPPERS_H

#include <control_block.h>

void scheduler_release_malloc();
void init_malloc();
void acquire_malloc();
void release_malloc();
void free_later(tcb_t *tcb);
void _free_tcb(tcb_t* tcb);
void _free_pcb(pcb_t* pcb);

#endif // KERN_INC_MALLOC_WRAPPERS_H