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
    NEW_LINK(pcb) all_processes;
    NEW_LINK(pcb) siblings;
    pcb_ds_t children;
    tcb_ds_t threads;
    int id;
    int parent_id;
    int reserved_pages;
    int exit_status;
    int num_threads;
    page_directory_t *directory;
    state_t state;
} pcb_t;

/** @brief Structure for a thread control block */
typedef struct tcb {
    NEW_LINK(tcb) all_threads;
    NEW_LINK(tcb) pcb_threads;
    int id;
    pcb_t *parent;
    void *kernel_stack;
    void *saved_esp;
    void *user_esp;
    state_t state;
} tcb_t;

/** @brief Structure for the overall kernel state */
typedef struct kernel_state {
    pcb_ds_t processes;
    tcb_ds_t threads;
    int next_pid;
    page_directory_t* dir;
} kernel_state_t;

// Headers regarding create process
kernel_state_t *get_kernel_state();
void init_kernel_state(page_directory_t* dir);
pcb_t *create_pcb_entry(pcb_t *parent_pcb);
tcb_t *create_tcb_entry(pcb_t *parent_pcb, void *stack);
void *allocate_kernel_stack();
void copy_kernel_stack(tcb_t *tcb_parent, tcb_t *tcb_child);
tcb_t *get_tcb_from_addr(void *addr);

#endif // CONTROL_H_