/** @file control.c
 *
 *  @brief Functions to create and manage PCBs and TCBs
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control.h>
#include <malloc.h>
#include <page.h>
#include <simics.h>
#include <string.h>
#include <switch.h>
#include <mutex.h>
#include <stack_info.h>
#include <cr.h>
#include <contracts.h>
#include <malloc_internal.h>

// Global kernel state with process and thread info
static kernel_state_t kernel_state;

/** @brief Initializes the global lists of processes and threads
 *  @return void
 **/
void init_kernel_state()
{
    INIT_STRUCT(&kernel_state.threads);
    kernel_state.next_id = 1;
    mutex_init(&kernel_state.next_id_mutex);
    mutex_init(&kernel_state.threads_mutex);
}

/** @brief Gives the next available process/thread id number
 *
 *  Note this does not take wrapping into account
 *
 *  @return Next sequential id
 **/
int get_next_id()
{
    mutex_lock(&kernel_state.next_id_mutex);
    int id = kernel_state.next_id;
    kernel_state.next_id++;
    mutex_unlock(&kernel_state.next_id_mutex);
    return id;
}

/** @brief Add a thread to the global list of threads
 *  @param tcb The thread to add
 *  @return void
 **/
void kernel_add_thread(tcb_t* tcb)
{
    mutex_lock(&kernel_state.threads_mutex);
    Q_INSERT_TAIL(&kernel_state.threads, tcb, all_threads);
    mutex_unlock(&kernel_state.threads_mutex);
}

/** @brief Remove a thread from the global list of threads
 *  @param tcb The thread to remove
 *  @return void
 **/
void kernel_remove_thread(tcb_t* tcb)
{
    mutex_lock(&kernel_state.threads_mutex);
    Q_REMOVE(&kernel_state.threads, tcb, all_threads);
    mutex_unlock(&kernel_state.threads_mutex);
}

/** @brief Add a thread to a process's thread list
 *  @param pcb The process to add the thread to
 *  @param tcb The thread to add
 *  @return void
 **/
void pcb_add_thread(pcb_t* pcb, tcb_t* tcb)
{
    mutex_lock(&pcb->threads_mutex);
    Q_INSERT_TAIL(&pcb->threads, tcb, pcb_threads);
    pcb->num_threads++;
    tcb->process = pcb;
    mutex_unlock(&pcb->threads_mutex);
}

/** @brief Remove a thread from a process's thread list
 *
 *  The process must have this thread in it's list or this function will
 *  produce undefined behavior
 *
 *  @param pcb The process to remove the thread from
 *  @param tcb The thread to remove
 *  @return The number of threads remaining in the process
 **/
int pcb_remove_thread(pcb_t* pcb, tcb_t* tcb)
{
    mutex_lock(&pcb->threads_mutex);
    Q_REMOVE(&pcb->threads, tcb, pcb_threads);
    pcb->num_threads--;
    int threads = pcb->num_threads;
    assert(threads >= 0);
    mutex_unlock(&pcb->threads_mutex);
    return threads;
}

/** @brief Add a child process to the parent process's list of children
 *
 *  @param parent The parent process
 *  @param child The child process
 *  @return void
 **/
void pcb_add_child(pcb_t* parent, pcb_t* child)
{
    mutex_lock(&child->parent_mutex);
    mutex_lock(&parent->children_mutex);
    Q_INSERT_TAIL(&parent->children, child, siblings);
    child->parent = parent;
    parent->num_children++;
    mutex_unlock(&parent->children_mutex);
    mutex_unlock(&child->parent_mutex);
}

/** @brief Creates a new pcb for a process and creates a tcb for the first
 *         thread
 *
 *  @return The tcb entry for the first thread of this process
 **/
tcb_t* create_pcb_entry()
{
    pcb_t* entry = (pcb_t*)smalloc(sizeof(pcb_t));
    if(entry == NULL){
        return NULL;
    }
    Q_INIT_ELEM(entry, siblings);
    mutex_init(&entry->parent_mutex);
    entry->parent = NULL;
    mutex_init(&entry->children_mutex);
    INIT_STRUCT(&entry->children);
    entry->num_children = 0;
    cond_init(&entry->wait);
    entry->waiting = 0;
    mutex_init(&entry->threads_mutex);
    INIT_STRUCT(&entry->threads);
    entry->num_threads = 0;
    entry->id = get_next_id();
    entry->exit_status = 0;
    // TODO; this is strange
    entry->state = P_ACTIVE;

    // create first process
    tcb_t* tcb = create_tcb_entry(entry->id);
    if (tcb == NULL) {
        free_pcb(entry);
        return NULL;
    }
    pcb_add_thread(entry, tcb);
    return tcb;
}

/** @brief Get the number of threads associated with a process
 *  @param pcb The process
 *  @return The number of threads associated with this process
 **/
int get_thread_count(pcb_t* pcb)
{
    mutex_lock(&pcb->threads_mutex);
    int thread_count = pcb->num_threads;
    assert(thread_count >= 0);
    mutex_unlock(&pcb->threads_mutex);
    return thread_count;
}

/** @brief Creates a new pcb entry for the current process
 *
 *  @param id The id of the thread this tcb will represent
 *  @return The new tcb entry
 **/
tcb_t* create_tcb_entry(int id)
{
    if(id < 0){
        lprintf("Thread id has wrapped, cannot create more threads");
        return NULL;
    }
    tcb_t* entry = (tcb_t*)smalloc(sizeof(tcb_t));
    if (entry == NULL) {
        return NULL;
    }
    uint32_t mem = (uint32_t)smemalign(K_STACK_SIZE, K_STACK_SIZE);
    if (mem == 0) {
        sfree(entry, sizeof(tcb_t));
        return NULL;
    }
    entry->kernel_stack = (void*)K_STACK_TOP(mem);
    // Store pointer to tcb at the top of the kernel stack
    *((tcb_t**)entry->kernel_stack) = entry;

    Q_INIT_ELEM(entry, all_threads);
    Q_INIT_ELEM(entry, pcb_threads);
    Q_INIT_ELEM(entry, runnable_threads);
    Q_INIT_ELEM(entry, suspended_threads);
    Q_INIT_ELEM(entry, sleeping_threads);
    entry->id = id;
    entry->state = T_NOT_YET;
    memset(&entry->swexn, 0, sizeof(swexn_t));
    entry->swexn.handler = NULL;
    entry->process = NULL;
    entry->wake_tick = 0;
    return entry;
}


/** @brief Free memory associated with a tcb_t structure without locks
 *
 *  @param tcb The tcb to free
 *  @return void
 **/
void _free_tcb(tcb_t* tcb)
{
    _sfree((void*)K_STACK_BASE(tcb->kernel_stack), K_STACK_SIZE);
    _sfree(tcb, sizeof(tcb_t));
}


/** @brief Free memory associated with a tcb_t structure
 *
 *  @param tcb The tcb to free
 *  @return void
 **/
void free_tcb(tcb_t* tcb)
{
    acquire_malloc();
    _free_tcb(tcb);
    release_malloc();
}

/** @brief Free memory associated with a pcb_t structure
 *
 *  @param pcb The pcb to free
 *  @return void
 **/
void _free_pcb(pcb_t* pcb)
{
    _sfree(pcb, sizeof(pcb_t));
}

/** @brief Free memory associated with a pcb_t structure
 *
 *  @param pcb The pcb to free
 *  @return void
 **/
void free_pcb(pcb_t* pcb)
{
    acquire_malloc();
    _free_pcb(pcb);
    release_malloc();
}

/** @brief Gets the tcb from the top of the kernel stack
 *
 *  @param An address on the current kernel stack
 *  @return Pointer to the tcb for the current kernel thread
 **/
tcb_t* get_tcb()
{
    uint32_t tcb_addr = K_STACK_TOP(K_STACK_BASE(get_esp()));
    tcb_t* tcb = *(tcb_t**)tcb_addr;
    ASSERT((uint32_t)(tcb->process->directory.dir) == get_cr3());
    return tcb;
}

/** @brief Get the tcb of a thread by the threads id
 *  @param tid The id of the thread to get
 *  @return The tcb of the thread
 **/
tcb_t* get_tcb_by_id(int tid)
{
    tcb_t* tcb;
    Q_FOREACH(tcb, &kernel_state.threads, all_threads)
    {
        if (tcb->id == tid && tcb->state != T_EXITED) {
            return tcb;
        }
    }
    return NULL;
}
