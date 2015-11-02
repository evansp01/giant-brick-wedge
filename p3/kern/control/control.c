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
#include <lmm/lmm.h>
#include <malloc/malloc_internal.h>
#include <page.h>
#include <simics.h>
#include <string.h>
#include <switch.h>
#include <mutex.h>

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

/** @brief Creates a new pcb entry for the current process
 *
 *  @param parent_pcb PCB entry for the parent process
 *  @return Pointer to the new tcb entry for the process thread
 **/
tcb_t *create_pcb_entry(pcb_t *parent_pcb)
{
    pcb_t *entry = (pcb_t *)malloc(sizeof(pcb_t));

    if (parent_pcb != NULL) {
        INIT_ELEM(entry, siblings);
        mutex_lock(&parent_pcb->children_mutex);
        INSERT(&parent_pcb->children, entry, siblings);
        mutex_unlock(&parent_pcb->children_mutex);
    }

    /* scheduler lists */
    INIT_STRUCT(&entry->children);
    INIT_STRUCT(&entry->threads);
    mutex_init(&entry->children_mutex);
    mutex_init(&entry->threads_mutex);

    entry->id = get_next_id();
    entry->exit_status = 0;
    entry->state = NOTYET;
    entry->num_threads = 0;
    mutex_init(&entry->num_threads_mutex);

    if (parent_pcb != NULL)
        entry->parent_id = parent_pcb->id;
    else
        entry->parent_id = 0;

    // create first process
    tcb_t *tcb = create_tcb_entry(entry);

    return tcb;
}

/** @brief Creates a new pcb entry for the current process
 *
 *  @param parent_pcb PCB entry for the parent process
 *  @param stack Pointer to the kernel stack for the thread
 *  @return Pointer to the new pcb entry
 **/
tcb_t *create_tcb_entry(pcb_t *parent_pcb)
{
    tcb_t *entry = (tcb_t *)smalloc(sizeof(tcb_t));
    
    uint32_t mem = (uint32_t)smemalign(PAGE_SIZE, PAGE_SIZE);
    if (mem == 0) {
        panic("Cannot allocate kernel stack");
    }
    void *stack = (void *)(mem + PAGE_SIZE - (2*sizeof(int)));

    Q_INIT_ELEM(entry, all_threads);
    Q_INIT_ELEM(entry, pcb_threads);
    Q_INIT_ELEM(entry, runnable_threads);
    
    // Scheduler thread list
    mutex_lock(&kernel_state.threads_mutex);
    INSERT(&kernel_state.threads, entry, all_threads);
    mutex_unlock(&kernel_state.threads_mutex);
    
    // Parent thread list
    mutex_lock(&parent_pcb->threads_mutex);
    INSERT(&parent_pcb->threads, entry, pcb_threads);
    mutex_unlock(&parent_pcb->threads_mutex);
    
    if (parent_pcb->num_threads == 0)
        entry->id = parent_pcb->id;
    else
        entry->id = get_next_id();
    
    mutex_lock(&parent_pcb->num_threads_mutex);
    parent_pcb->num_threads++;
    mutex_unlock(&parent_pcb->num_threads_mutex);
    
    entry->parent = parent_pcb;
    entry->kernel_stack = stack;
    entry->state = NOTYET;

    // Store pointer to tcb at the top of the kernel stack
    *((tcb_t **)stack) = entry;
    
    return entry;
}


/** @brief Gets the tcb from the top of the kernel stack
 *
 *  @param An address on the current kernel stack
 *  @return Pointer to the tcb for the current kernel thread
 **/
tcb_t *get_tcb()
{
    uint32_t tcb_addr = (get_esp()&0xFFFFF000)|0xFF8;
    return *(tcb_t **)tcb_addr;
}
