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

/** @brief Thread exit states */
typedef enum {
    THREAD_EXIT_SUCCESS,
    THREAD_EXIT_FAILED
} thread_exit_state_t;

/** @brief Thread states */
typedef enum {
    T_NOT_YET,
    T_RUNNABLE_P0,
    T_RUNNABLE_P1,
    T_SUSPENDED,
    T_KERN_SUSPENDED,
    T_SLEEPING,
    T_EXITED
} thread_state_t;

/** @brief Process states */
typedef enum {
    P_EXITED,
    P_ACTIVE
} process_state_t;

/** @brief Function pointer for swexn handler */
typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);

/** @brief Struct for swexn syscall */
typedef struct swexn {
    swexn_handler_t handler;
    void *arg;
    void *stack;
} swexn_t;

/** @brief Struct of stack data swexn syscall */
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
    ppd_t *directory;
    process_state_t state;
} pcb_t;

/** @brief Structure for a thread control block */
typedef struct tcb {
    Q_NEW_LINK(tcb) all_threads;
    Q_NEW_LINK(tcb) pcb_threads;
    Q_NEW_LINK(tcb) runnable_threads;
    Q_NEW_LINK(tcb) suspended_threads;
    Q_NEW_LINK(tcb) sleeping_threads;
    int id;
    pcb_t *process;
    void *kernel_stack;
    void *saved_esp;
    ppd_t *free_pointer;
    thread_state_t state;
    swexn_t swexn;
    unsigned int wake_tick;
} tcb_t;

/** @brief Structure for the overall kernel state */
typedef struct kernel_state {
    mutex_t threads_mutex;
    tcb_ds_t threads;
    mutex_t next_id_mutex;
    int next_id;
    tcb_t *init;
} kernel_state_t;

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

#endif // KERN_INC_CONTROL_BLOCK_H