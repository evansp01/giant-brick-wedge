/** @file control_block_struct.h
 *  @brief Interface for process and thread control blocks
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_CONTROL_BLOCK_STRUCTS_H
#define KERN_INC_CONTROL_BLOCK_STRUCTS_H

#include <cond.h>
#include <ureg.h>
#include <vm.h>
#include <mutex.h>
#include <variable_queue.h>
#include <variable_htable.h>
#include <user_drivers.h>

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
Q_NEW_HEAD(pcb_queue_t, pcb);
/** @brief Structure for a list of threads */
Q_NEW_HEAD(tcb_queue_t, tcb);
/** @brief Structure for hash table of tcbs */
H_NEW_TABLE(thread_hash_t, tcb_queue_t);

/** @brief Structure for a process control block */
typedef struct pcb {
    Q_NEW_LINK(pcb) siblings;
    // Stuff protected by parent mutex
    mutex_t parent_mutex;
    struct pcb *parent;
    // Stuff protected by children_mutex
    mutex_t children_mutex;
    pcb_queue_t children;
    int num_children;
    cond_t wait;
    int waiting;
    // Stuff protected by threads_mutex
    mutex_t threads_mutex;
    tcb_queue_t threads;
    int num_threads;
    // Things not protected by a mutex
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
    devserv_list_t devserv;
} tcb_t;

/** @brief Structure for the overall kernel state */
typedef struct kernel_state {
    mutex_t threads_mutex;
    thread_hash_t thread_table;
    mutex_t next_id_mutex;
    int next_id;
    tcb_t *init;
} kernel_state_t;

#endif // KERN_INC_CONTROL_BLOCK_STRUCTS_H
