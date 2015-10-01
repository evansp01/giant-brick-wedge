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
#include <variable_queue.h>

typedef struct TCB {
    Q_NEW_LINK(TCB) link;
    void *stack;
    void *exit_val;
    int tid;
    int status;
    int joining;
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
    void *stack_low, *stack_high;
    get_stack_bounds(&stack_high, &stack_low);
    frame_alloc_init(size, stack_high, stack_low);
    MAGIC_BREAK;
    //something about malloc?
    //mutexes??????
    thread_info.base_tid = gettid();
    Q_INIT_HEAD(&thread_info.TCB_list);
    mutex_init(&thread_info.TCB_mutex);
    return 0;
}

void thr_wrapper(void *(*func)(void*), void *arg, int *stack_base)
{
    void *base = stack_base;
    int tid = gettid();
    *stack_base = tid;
    
    // Add TCB entry for current node if it does not already exist
    add_TCB_entry(base, tid);
    
    void *status = func(arg);
    thr_exit(status);
}

int thr_join(int tid, void** statusp)
{
    
    
    return 0;
}

void thr_exit(void* status)
{
    //put exit status in tcb
    //tcb_add_exit(gettid(), status);
    //get my stack pointer
    //free_and_vanish
    void *stack = NULL;
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


void add_TCB_entry(void *stack, int tid)
{
    // Acquire TCB list mutex
    mutex_lock(&thread_info.TCB_mutex);
    
    // Check if TCB entry has already been created
    TCB_t* cur;
    Q_FOREACH(cur, &thread_info.TCB_list, link) {      
        if (cur->tid == tid) {
            mutex_unlock(&thread_info.TCB_mutex);
            return;
        }
    }
    
    // Otherwise create TCB entry
    TCB_t* node = (TCB_t*)malloc(sizeof(TCB_t));
    Q_INIT_ELEM(node, link);
    node->stack = stack;
    node->tid = tid;
    node->exit_val = NULL;
    node->status = 0;
    node->joining = 0;
    Q_INSERT_TAIL(&thread_info.TCB_list, node, link);
    
    // Release TCB list mutex
    mutex_unlock(&thread_info.TCB_mutex);
}
