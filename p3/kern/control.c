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
#include <datastructures/variable_queue.h>
#include <lmm/lmm.h>
#include <malloc/malloc_internal.h>


// Global kernel state with process and thread info
static kernel_state_t kernel_state;


/** @brief Initializes the global lists of processes and threads
 *
 *  @return Zero on success, an integer less than zero on failure
 **/
int init_kernel_state()
{
    Q_INIT_HEAD(&kernel_state.processes);
    Q_INIT_HEAD(&kernel_state.threads);
    kernel_state.next_id = 1;
    return 0;
}

/** @brief Gives the next available process id number
 *
 *  @return Next sequential process id
 **/
int get_next_process_id()
{
    int id = kernel_state.next_id;
    kernel_state.next_id++;
    return id;
}

/** @brief Creates a new pcb entry for the current process
 *
 *  @param parent_pcb PCB entry for the parent process
 *  @return Pointer to the new pcb entry
 **/
pcb_t *create_pcb_entry(pcb_t *parent_pcb)
{
    pcb_t *entry = (pcb_t *)malloc(sizeof(pcb_t));
    
    Q_INIT_ELEM(entry, all_processes);
    Q_INSERT_FRONT(&kernel_state.processes, entry, all_processes);
    
    if (parent_pcb != NULL) {
        Q_INIT_ELEM(entry, siblings);
        Q_INSERT_FRONT(&parent_pcb->children, entry, siblings);
    }
    
    /* scheduler lists */
    Q_INIT_HEAD(&entry->children);
    Q_INIT_HEAD(&entry->threads);
    
    entry->id = get_next_process_id();
    entry->exit_status = 0;
    entry->state = NOTYET;
    
    if (parent_pcb != NULL)
        entry->parent_id = parent_pcb->id;
    else
        entry->parent_id = 0;
    
    entry->reserved_pages = 0;
    entry->directory = NULL;
    
    return entry;
}

/** @brief Creates a new pcb entry for the current process
 *
 *  @param parent_pcb PCB entry for the parent process
 *  @param stack Pointer to the kernel stack for the thread
 *  @return Pointer to the new pcb entry
 **/
tcb_t *create_tcb_entry(pcb_t *parent_pcb, void *stack)
{
    tcb_t *entry = (tcb_t *)malloc(sizeof(tcb_t));
    
    Q_INIT_ELEM(entry, all_threads);
    Q_INIT_ELEM(entry, pcb_threads);
    
    /* scheduler lists */
    Q_INSERT_FRONT(&kernel_state.threads, entry, all_threads);
    Q_INSERT_FRONT(&parent_pcb->threads, entry, pcb_threads);
    
    entry->id = 0;
    entry->pid = parent_pcb->id;
    entry->kernel_stack = stack;
    entry->state = NOTYET;
    
    return entry;
}

/** @brief Creates a new kernel stack for a kernel process
 *
 *  @return Pointer to the top of the new kernel stack
 **/
void *allocate_kernel_stack()
{
    // TODO: Change if lmm region flags are used
    char *mem = (char *)lmm_alloc_page(&malloc_lmm, 0);
    
    // TODO: Is the pointer math correct?
    return (void *)(mem + PAGE_SIZE - 1);
}

