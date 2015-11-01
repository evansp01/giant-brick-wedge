/** @file control.h
 *  @brief Interface for process and thread control blocks
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef CONTROL_H_
#define CONTROL_H_

#include <vm.h>
#include <interface.h>
#include <mutex.h>

typedef enum state {
    NOTYET,
    RUNNABLE,
    EXITED
} state_t;

/** @brief Structure for a list of processes */
NEW_STRUCT(pcb_ds_t, pcb);
/** @brief Structure for a list of threads */
NEW_STRUCT(tcb_ds_t, tcb);

/** @brief Structure for a process control block */
typedef struct pcb {
    NEW_LINK(pcb) siblings;
    pcb_ds_t children;
    tcb_ds_t threads;
    mutex_t children_mutex;
    mutex_t threads_mutex;
    int id;
    int parent_id;
    int exit_status;
    int num_threads;
    mutex_t num_threads_mutex;
    ppd_t directory;
    state_t state;
} pcb_t;

/** @brief Structure for a thread control block */
typedef struct tcb {
    NEW_LINK(tcb) all_threads;
    NEW_LINK(tcb) pcb_threads;
    NEW_LINK(tcb) runnable_threads;
    int id;
    pcb_t *parent;
    void *kernel_stack;
    void *saved_esp;
    state_t state;
} tcb_t;

/** @brief Structure for the overall kernel state */
typedef struct kernel_state {
    tcb_ds_t threads;
    int next_id;
    mutex_t next_id_mutex;
    mutex_t threads_mutex;
} kernel_state_t;

// Headers regarding create process
kernel_state_t *get_kernel_state();
void init_kernel_state();
tcb_t *create_pcb_entry(pcb_t *parent_pcb);
tcb_t *create_tcb_entry(pcb_t *parent_pcb);
void *allocate_kernel_stack();
void copy_kernel_stack(tcb_t *tcb_parent, tcb_t *tcb_child);
tcb_t *get_tcb();

#endif // CONTROL_H_
