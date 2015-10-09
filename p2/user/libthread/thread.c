/** @file thread.c
 *  @brief Implementation of the thread functions.
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/
 
#include <thr_internals.h>
#include <stdlib.h>
#include <syscall.h>
#include <thread.h>
#include <simics.h>
#include <mutex.h>
#include <cond.h>
#include <variable_queue.h>
#include <autostack.h>
#include <errors.h>

enum thread_status {
    NOTYET,
    RUNNING,
    EXITED
};

/** @brief tcb struct */
typedef struct tcb {
    Q_NEW_LINK(tcb) link;
    void* stack;
    void* exit_val;
    int tid;
    volatile int joining;
    volatile enum thread_status status;
    cond_t cvar;
    mutex_t mutex;
} tcb_t;

Q_NEW_HEAD(tcb_list_t, tcb);

/** @brief A struct for keeping track of threads */
typedef struct thread_info_t {
    mutex_t tcb_mutex;
    tcb_list_t tcb_list;
    int base_tid;
} thread_info_t;

static thread_info_t thread_info;

tcb_t* get_tcb_entry(int tid);
tcb_t* get_locked_tcb_entry(int tid);
tcb_t* create_tcb_entry(void* stack, int tid);

/** @brief Initialize the multi-threaded environment
 *
 *  Sets future thread stacks to be of the given size and ensures that the
 *  current thread is at least the same size. Initializes the tcb list and
 *  its required mutexes.
 *
 *  @param size The size of frames
 *  @return zero on success and less than zero on failure
 **/
int thr_init(unsigned int size)
{
    install_threaded();
    void* stack_low, *stack_high;
    get_stack_bounds(&stack_high, &stack_low);
    if(frame_alloc_init(size, stack_high, stack_low) < 0){
        return -1;
    }
    thread_info.base_tid = gettid();
    Q_INIT_HEAD(&thread_info.tcb_list);
    mutex_init(&thread_info.tcb_mutex);
    tcb_t* entry = create_tcb_entry(stack_high, thread_info.base_tid);
    Q_INSERT_TAIL(&thread_info.tcb_list, entry, link);
    initialize_malloc();
    threaded_exit();
    return 0;
}

/** @brief Main wrapper for new threads
 *
 *  Runs the given function with arguments in the new thread.
 *
 *  @param func Function pointer for thread to run
 *  @param arg Arguments to the function
 *  @param stack_base The address of the stack base
 *  @return Void
 **/
void thr_wrapper(void* (*func)(void*), void* arg, int* stack_base)
{
    //must use system call since tid is not yet on stack
    int tid = gettid();
    void* base = stack_base;
    *stack_base = tid;

    // Add tcb entry for current entry if it does not already exist
    ensure_tcb_exists(base, tid);

    install_threaded();
    void* status = func(arg);
    thr_exit(status);
}

/** @brief Joins the given thread once it has exited
 *
 *  Joins the given thread as long as join has not already been called on it
 *  by another thread. If the thread has not yet exited the function blocks.
 *  Returns the exit status once the thread has exited.
 *
 *  @param tid Thread id of the thread to join
 *  @param statusp Pointer to return the exit status of the thread
 *  @return zero on success and less than zero on failure
 **/
int thr_join(int tid, void** statusp)
{
    tcb_t* entry = get_locked_tcb_entry(tid);
    if (entry == NULL) {
        return -1;
    }
    if (entry->status == NOTYET || entry->joining) {
        mutex_unlock(&entry->mutex);
        return -1;
    }
    //we have the lock for the entry
    entry->joining = 1;
    if (entry->status != EXITED) {
        cond_wait(&entry->cvar, &entry->mutex);
    }
    if (entry->status != EXITED) {
        EXIT_ERROR("Joiner signaled, but thread has not exited");
    }
    //now that the entry is joining, we should be okay
    mutex_unlock(&entry->mutex);
    // we have no locks at all!
    mutex_lock(&thread_info.tcb_mutex);
    Q_REMOVE(&thread_info.tcb_list, entry, link);
    mutex_unlock(&thread_info.tcb_mutex);
    if (statusp != NULL) {
        *statusp = entry->exit_val;
    }
    cond_destroy(&entry->cvar);
    mutex_destroy(&entry->mutex);
    free(entry);
    return 0;
}

/** @brief Exits the thread and cleans up the thread stack
 *
 *  @param status Exit status for the thread
 *  @return Void
 **/
void thr_exit(void* status)
{
    tcb_t* entry = get_locked_tcb_entry(thr_getid());
    if (entry == NULL) {
        EXIT_ERROR("Thread %d exiting has no tcb entry", gettid());
    }
    entry->exit_val = status;
    entry->status = EXITED;
    void* stack = entry->stack;
    if (entry->joining) {
        cond_signal(&entry->cvar);
    }
    mutex_unlock(&entry->mutex);
    free_frame_and_vanish(stack);
}

/** @brief Retrieves the thread id
 *
 *  @return Thread id
 **/
int thr_getid(void)
{
    int* stack;
    switch (get_address_stack(get_esp(), (void**)&stack)) {
    case FIRST_STACK:
        return thread_info.base_tid;
    case THREAD_STACK:
        return *stack;
    case UNALLOCATED_PAGE:
    case NOT_ON_STACK:
        break;
    }
    // we don't know who you are. The system does though!
    return gettid();
}

/** @brief Yields to the given thread
 *
 *  @param tid Thread id of thread to yield to
 *  @return zero on success and less than zero on failure
 **/
int thr_yield(int tid)
{
    return yield(tid);
}

/** @brief Retrieves tcb entry for the given thread
 *
 *  This implementation uses mutexes to guarantee the state of the tcb list
 *  and entries.
 *
 *  @param tid Thread id of thread to retrieve
 *  @return tcb entry if found, null otherwise
 **/
tcb_t* get_locked_tcb_entry(int tid)
{
    tcb_t* entry;
    mutex_lock(&thread_info.tcb_mutex);
    Q_FOREACH(entry, &thread_info.tcb_list, link)
    {
        if (entry->tid == tid) {
            mutex_lock(&entry->mutex);
            mutex_unlock(&thread_info.tcb_mutex);
            return entry;
        }
    }
    mutex_unlock(&thread_info.tcb_mutex);
    return NULL;
}

/** @brief Retrieves tcb entry for the given thread
 *
 *  @param tid Thread id of thread to retrieve
 *  @return tcb entry if found, null otherwise
 **/
tcb_t* get_tcb_entry(int tid)
{
    tcb_t* entry;
    Q_FOREACH(entry, &thread_info.tcb_list, link)
    {
        if (entry->tid == tid) {
            return entry;
        }
    }
    return NULL;
}

/** @brief Function to check that tcb entry for new thread exists
 *
 *  This function is called from both the parent and child thread. If the
 *  entry does not yet exist, the function will create it and wait for the
 *  other thread to acknowledge. If the entry already exists the thread will
 *  signal the other thread.
 *
 *  @param stack Address of new thread stack
 *  @param tid Thread id of created thread
 *  @return Void
 **/
void ensure_tcb_exists(void* stack, int tid)
{
    // Acquire tcb list mutex
    // Check if tcb entry has already been created
    mutex_lock(&thread_info.tcb_mutex);
    tcb_t* entry = get_tcb_entry(tid);
    if (entry != NULL) {
        mutex_lock(&entry->mutex);
        mutex_unlock(&thread_info.tcb_mutex);
        //acknowledge that the thread exists
        entry->status = RUNNING;
        //drop the mutex
        mutex_unlock(&entry->mutex);
        //allow the other guy to wake up
        cond_signal(&entry->cvar);
        return;
    }
    entry = create_tcb_entry(stack, tid);
    Q_INSERT_TAIL(&thread_info.tcb_list, entry, link);
    mutex_lock(&entry->mutex);
    mutex_unlock(&thread_info.tcb_mutex);
    //wait for other thread to acknowledge
    cond_wait(&entry->cvar, &entry->mutex);
    mutex_unlock(&entry->mutex);
    // Release tcb list mutex
}

/** @brief Function to create a new tcb entry for the current thread
 *
 *  @param stack Address of new thread stack
 *  @param tid Thread id of created thread
 *  @return pointer to new tcb entry
 **/
tcb_t* create_tcb_entry(void* stack, int tid)
{
    tcb_t* entry = (tcb_t*)malloc(sizeof(tcb_t));
    Q_INIT_ELEM(entry, link);
    entry->tid = tid;
    entry->stack = stack;
    entry->joining = 0;
    entry->exit_val = NULL;
    entry->status = NOTYET;
    cond_init(&entry->cvar);
    mutex_init(&entry->mutex);
    return entry;
}
