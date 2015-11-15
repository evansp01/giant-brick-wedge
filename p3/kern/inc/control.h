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
#include <cond.h>
#include <setup_idt.h>
#include <ureg.h>

typedef enum {
    NOT_YET,
    RUNNABLE,
    SUSPENDED,
    EXITED
} state_t;

typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);
typedef struct swexn {
    swexn_handler_t handler;
    void *arg;
    void *stack;
} swexn_t;

typedef struct swexn_stack {
    void *ret_addr;
    void *arg;
    void *ureg;
    ureg_t state;
} swexn_stack_t;

/** @brief Structure for a list of processes */
Q_NEW_HEAD(pcb_ds_t, pcb);
/** @brief Structure for a list of threads */
Q_NEW_HEAD(tcb_ds_t, tcb);

/** @brief Structure for a process control block */
typedef struct pcb {
    Q_NEW_LINK(pcb) siblings;
    // Stuff protected by parent mutex
    mutex_t parent_mutex;
    struct pcb *parent;
    // Stuff protected by children_mutex
    mutex_t children_mutex;
    pcb_ds_t children;
    int num_children;
    cond_t wait;
    int waiting;
    // Stuff protected by threads_mutex
    mutex_t threads_mutex;
    tcb_ds_t threads;
    int num_threads;
    // Other things
    int id;
    int exit_status;
    ppd_t directory;
    state_t state;
} pcb_t;

/** @brief Structure for a thread control block */
typedef struct tcb {
    Q_NEW_LINK(tcb) all_threads;
    Q_NEW_LINK(tcb) pcb_threads;
    Q_NEW_LINK(tcb) runnable_threads;
    Q_NEW_LINK(tcb) suspended_threads;
    int id;
    pcb_t *process;
    void *kernel_stack;
    void *saved_esp;
    state_t state;
    swexn_t swexn;
    unsigned int wake_tick;
} tcb_t;

/** @brief Structure for the overall kernel state */
typedef struct kernel_state {
    mutex_t threads_mutex;
    tcb_ds_t threads;
    mutex_t next_id_mutex;
    int next_id;
} kernel_state_t;

// Headers regarding create process
kernel_state_t *get_kernel_state();
void init_kernel_state();
tcb_t *create_pcb_entry();
void free_pcb(pcb_t* pcb);
tcb_t *create_tcb_entry(int id);
void free_tcb(tcb_t* tcb);
void *allocate_kernel_stack();
void copy_kernel_stack(tcb_t *tcb_parent, tcb_t *tcb_child);
tcb_t *get_tcb();
tcb_t* get_tcb_by_id(int tid);
int get_thread_count(pcb_t *pcb);
void finalize_exit(tcb_t *tcb);
void vanish_thread(tcb_t *tcb);
int wait(pcb_t* pcb, int *status_ptr);
void pcb_add_child(pcb_t *parent, pcb_t *child);
int pcb_remove_thread(pcb_t* pcb, tcb_t* tcb);
void pcb_add_thread(pcb_t* pcb, tcb_t* tcb);
void kernel_remove_thread(tcb_t* tcb);
void kernel_add_thread(tcb_t* tcb);
pcb_t *thread_exit(tcb_t *tcb);
void acquire_malloc();
void release_malloc();

#endif // CONTROL_H_
