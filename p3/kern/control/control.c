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

// Global kernel state with process and thread info
kernel_state_t kernel_state;

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

void kernel_add_thread(tcb_t* tcb)
{
    mutex_lock(&kernel_state.threads_mutex);
    Q_INSERT_TAIL(&kernel_state.threads, tcb, all_threads);
    mutex_unlock(&kernel_state.threads_mutex);
}

void kernel_remove_thread(tcb_t* tcb)
{
    mutex_lock(&kernel_state.threads_mutex);
    Q_REMOVE(&kernel_state.threads, tcb, all_threads);
    mutex_unlock(&kernel_state.threads_mutex);
}

void pcb_add_thread(pcb_t* pcb, tcb_t* tcb)
{
    mutex_lock(&pcb->threads_mutex);
    Q_INSERT_TAIL(&pcb->threads, tcb, pcb_threads);
    pcb->num_threads++;
    tcb->parent = pcb;
    mutex_unlock(&pcb->threads_mutex);
}

void pcb_remove_thread(pcb_t* pcb, tcb_t* tcb)
{
    mutex_lock(&pcb->threads_mutex);
    Q_REMOVE(&pcb->threads, tcb, pcb_threads);
    pcb->num_threads--;
    mutex_unlock(&pcb->threads_mutex);
}

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

/** @brief Creates a new pcb entry for the current process
 *
 *  @param parent_pcb PCB entry for the parent process
 *  @return Pointer to the new tcb entry for the process thread
 **/
tcb_t* create_pcb_entry()
{
    pcb_t* entry = (pcb_t*)malloc(sizeof(pcb_t));
    Q_INIT_ELEM(entry, siblings);

    /* scheduler lists */
    INIT_STRUCT(&entry->children);
    INIT_STRUCT(&entry->threads);
    mutex_init(&entry->parent_mutex);
    mutex_init(&entry->children_mutex);
    mutex_init(&entry->threads_mutex);
    cond_init(&entry->wait);

    entry->id = get_next_id();
    entry->exit_status = 0;
    // TODO; this is strange
    entry->state = RUNNABLE;
    entry->num_threads = 0;
    entry->num_children = 0;
    entry->parent = NULL;

    // create first process
    tcb_t* tcb = create_tcb_entry(entry->id);
    if (tcb == NULL) {
        free(entry);
        return NULL;
    }
    pcb_add_thread(entry, tcb);
    return tcb;
}

int get_thread_count(pcb_t* pcb)
{
    mutex_lock(&pcb->threads_mutex);
    int thread_count = pcb->num_threads;
    mutex_unlock(&pcb->threads_mutex);
    return thread_count;
}

/** @brief Creates a new pcb entry for the current process
 *
 *  @param parent_pcb PCB entry for the parent process
 *  @param stack Pointer to the kernel stack for the thread
 *  @return Pointer to the new pcb entry
 **/
tcb_t* create_tcb_entry(int id)
{
    tcb_t* entry = (tcb_t*)smalloc(sizeof(tcb_t));
    if (entry == NULL) {
        return NULL;
    }
    uint32_t mem = (uint32_t)smemalign(K_STACK_SIZE, K_STACK_SIZE);
    if (mem == 0) {
        free(entry);
        return NULL;
    }
    entry->kernel_stack = (void*) K_STACK_TOP(mem);
    // Store pointer to tcb at the top of the kernel stack
    *((tcb_t**)entry->kernel_stack) = entry;

    Q_INIT_ELEM(entry, all_threads);
    Q_INIT_ELEM(entry, pcb_threads);
    Q_INIT_ELEM(entry, runnable_threads);
    entry->id = id;
    entry->state = SUSPENDED;
    return entry;
}

void free_tcb(tcb_t* tcb)
{
    sfree(tcb->kernel_stack, K_STACK_SIZE);
    free(tcb);
}

void free_pcb(pcb_t* pcb)
{
    free(pcb);
}

/** @brief Gets the tcb from the top of the kernel stack
 *
 *  @param An address on the current kernel stack
 *  @return Pointer to the tcb for the current kernel thread
 **/
tcb_t* get_tcb()
{
    uint32_t tcb_addr = K_STACK_TOP(K_STACK_BASE(get_esp()));
    return *(tcb_t**)tcb_addr;
}

tcb_t* get_tcb_by_id(int tid)
{
    tcb_t* tcb;
    Q_FOREACH(tcb, &kernel_state.threads, all_threads)
    {
        if (tcb->id == tid && tcb->state != EXITED) {
            return tcb;
        }
    }
    return NULL;
}
