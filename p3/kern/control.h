/** @file control.h
 *  @brief Interface for process and thread control blocks
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef CONTROL_H_
#define CONTROL_H_

#include <vm.h>
#include <datastructures/interface.h>

typedef enum state {
    NOTYET,
    RUNNING,
    EXITED
} state_t;

// Declare the data structure
NEW_STRUCT(pcb_ds_t, pcb);
NEW_STRUCT(tcb_ds_t, tcb);

/** @brief Structure for a process control block */
typedef struct pcb {
    // Links for linked list macros
    NEW_LINK(pcb) all_processes;
    NEW_LINK(pcb) siblings;
   
   /* scheduler lists */
    pcb_ds_t children;
    tcb_ds_t threads;
    int id;
    int parent_id;
    int reserved_pages;
    int exit_status;
    int next_tid;
    page_directory_t *directory;
    state_t state;
} pcb_t;

/** @brief Structure for a thread control block */
typedef struct tcb {
    /* scheduler lists */
    NEW_LINK(tcb) all_threads;
    NEW_LINK(tcb) pcb_threads;
    
    int id;
    pcb_t *parent;
    void *kernel_stack;
    state_t state;
} tcb_t;

/** @brief Structure for the overall kernel state */
typedef struct kernel_state {
    pcb_ds_t processes;
    tcb_ds_t threads;
    int next_pid;
} kernel_state_t;

// Headers regarding create process
int init_kernel_state();
pcb_t *create_pcb_entry(pcb_t *parent_pcb);
tcb_t *create_tcb_entry(pcb_t *parent_pcb, void *stack);
void *allocate_kernel_stack();

#endif // CONTROL_H_