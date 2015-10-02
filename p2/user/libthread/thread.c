/** @file thread.c
 *  @brief This file defines thread-management interface.
 *
 *  It should NOT contain any of the thread library internals.
 *  Therefore, you may NOT modify this file.
 *
 *  However, you MAY modify user/libthread/thr_internals.h.
 *
 */
#include <thr_internals.h>
#include <stdlib.h>
#include <autostack.h>
#include <syscall.h>
#include <thread.h>
#include <simics.h>
#include <mutex.h>
#include <cond.h>
#include <variable_queue.h>

enum thread_status {
    EXITED,
    RUNNING
};

typedef struct TCB {
    Q_NEW_LINK(TCB) link;
    void* stack;
    void* exit_val;
    int tid;
    cond_t cvar;
    volatile int joining;
    volatile enum thread_status status;
} TCB_t;

Q_NEW_HEAD(TCB_list_t, TCB);

/** @brief A struct for keeping track of threads */
typedef struct thread_info_t {
    mutex_t TCB_mutex;
    TCB_list_t TCB_list;
    int base_tid;
} thread_info_t;

/** @brief Thread info struct */
static thread_info_t thread_info;

/* thread library functions */
int thr_init(unsigned int size)
{
    install_threaded();
    void* stack_low, *stack_high;
    get_stack_bounds(&stack_high, &stack_low);
    frame_alloc_init(size, stack_high, stack_low);
    thread_info.base_tid = gettid();
    Q_INIT_HEAD(&thread_info.TCB_list);
    mutex_init(&thread_info.TCB_mutex);
    add_TCB_entry(stack_high, thread_info.base_tid);
    initialize_malloc();
    MAGIC_BREAK;
    return 0;
}

TCB_t* get_TCB_entry(int tid);

void thr_wrapper(void* (*func)(void*), void* arg, int* stack_base)
{
    void* base = stack_base;
    int tid = thr_getid();
    *stack_base = tid;

    // Add TCB entry for current node if it does not already exist
    add_TCB_entry(base, tid);

    void* status = func(arg);
    thr_exit(status);
}

int thread_join_helper(TCB_t* entry, void** statusp)
{
    *statusp = entry->exit_val;
    Q_REMOVE(&thread_info.TCB_list, entry, link);
    free(entry);
    return 0;
}

int thr_join(int tid, void** statusp)
{
    mutex_lock(&thread_info.TCB_mutex);
    TCB_t* entry = get_TCB_entry(tid);
    if (entry == NULL || entry->joining) {
        mutex_unlock(&thread_info.TCB_mutex);
        return -1;
    }
    if (entry->status == EXITED) {
        int status = thread_join_helper(entry, statusp);
        mutex_unlock(&thread_info.TCB_mutex);
        return status;
    }
    entry->joining = 1;
    cond_wait(&entry->cvar, &thread_info.TCB_mutex);
    if (entry->status != EXITED) {
        lprintf("Joiner was signaled, but thread has not exited, oh no!\n");
    }
    int status = thread_join_helper(entry, statusp);
    mutex_unlock(&thread_info.TCB_mutex);
    return status;
}

void thr_exit(void* status)
{
    mutex_lock(&thread_info.TCB_mutex);
    TCB_t* entry = get_TCB_entry(thr_getid());
    if (entry == NULL) {
        lprintf("Thread exiting does not have tcb entry. Probably not good\n");
    }
    entry->exit_val = status;
    entry->status = EXITED;
    if (entry->joining) {
        cond_signal(&entry->cvar);
    }
    void* stack = entry->stack;
    mutex_unlock(&thread_info.TCB_mutex);
    free_frame_and_vanish(stack);
}

int thr_getid(void)
{
    // TODO: Get tid for base thread

    // TODO: Get tid from stack if not in base thread

    // Get system tid if other options fail
    return gettid();
}

int thr_yield(int tid)
{
    return yield(tid);
}

TCB_t* get_TCB_entry(int tid)
{
    TCB_t* cur;
    Q_FOREACH(cur, &thread_info.TCB_list, link)
    {
        if (cur->tid == tid) {
            return cur;
        }
    }
    return NULL;
}

void add_TCB_entry(void* stack, int tid)
{
    // Acquire TCB list mutex
    mutex_lock(&thread_info.TCB_mutex);
    // Check if TCB entry has already been created
    TCB_t* entry = get_TCB_entry(tid);
    if (entry != NULL) {
        mutex_unlock(&thread_info.TCB_mutex);
        return;
    }
    // Otherwise create TCB entry
    TCB_t* node = (TCB_t*)malloc(sizeof(TCB_t));
    Q_INIT_ELEM(node, link);
    node->stack = stack;
    node->tid = tid;
    node->exit_val = NULL;
    node->status = RUNNING;
    node->joining = 0;
    cond_init(&node->cvar);
    Q_INSERT_TAIL(&thread_info.TCB_list, node, link);
    // Release TCB list mutex
    mutex_unlock(&thread_info.TCB_mutex);
}
