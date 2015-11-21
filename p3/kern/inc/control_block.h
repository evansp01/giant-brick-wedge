/** @file control_block.h
 *  @brief Interface for process and thread control blocks
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_CONTROL_BLOCK_H
#define KERN_INC_CONTROL_BLOCK_H

#include <vm.h>
#include <mutex.h>
#include <cond.h>
#include <ureg.h>
#include <control_block_struct.h>

extern kernel_state_t kernel_state;
// Headers regarding create process
void init_kernel_state();
tcb_t *create_pcb_entry();
void free_pcb(pcb_t* pcb);
tcb_t *create_tcb_entry(int id);
void free_tcb(tcb_t* tcb);
void copy_kernel_stack(tcb_t *tcb_parent, tcb_t *tcb_child);
tcb_t *get_tcb();
tcb_t* get_tcb_by_id(int tid);
int get_thread_count(pcb_t *pcb);
void vanish_thread(tcb_t *tcb, thread_exit_state_t failed);
int wait(pcb_t* pcb, int *status_ptr);
void pcb_add_child(pcb_t *parent, pcb_t *child);
int pcb_remove_thread(pcb_t* pcb, tcb_t* tcb);
void pcb_add_thread(pcb_t* pcb, tcb_t* tcb);
void kernel_remove_thread(tcb_t* tcb);
void kernel_add_thread(tcb_t* tcb);
int get_next_id();
void finalize_exit(tcb_t *tcb);

/** @brief Get the current value of esp
 *  @return The value of esp
 **/
uint32_t get_esp();

#endif // KERN_INC_CONTROL_BLOCK_H
