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


// Global kernel state with process and thread info
static kernel_state_t kernel_state;


/** @brief Initializes the global lists of processes and threads
 *
 *  @return Zero on success, an integer less than zero on failure
 **/
int init_kernel_state()
{
    INIT_STRUCT(&kernel_state.processes);
    INIT_STRUCT(&kernel_state.threads);
    kernel_state.next_pid = 1;
    return 0;
}

/** @brief Gives the next available process id number
 *
 *  @return Next sequential process id
 **/
int get_next_pid()
{
    int id = kernel_state.next_pid;
    kernel_state.next_pid++;
    return id;
}

/** @brief Gives the next available thread id for a particular process
 *
 *  @param parent_pcb Pointer to the pcb of the thread's parent process
 *  @return Thread id on success, an integer less than zero on failure
 **/
int get_next_tid(pcb_t *parent_pcb)
{
    if (parent_pcb == NULL)
        return -1;
    int id = parent_pcb->next_tid;
    parent_pcb->next_tid++;
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

    INIT_ELEM(entry, all_processes);
    INSERT(&kernel_state.processes, entry, all_processes);

    if (parent_pcb != NULL) {
        INIT_ELEM(entry, siblings);
        INSERT(&parent_pcb->children, entry, siblings);
    }

    /* scheduler lists */
    INIT_STRUCT(&entry->children);
    INIT_STRUCT(&entry->threads);

    entry->id = get_next_pid();
    entry->exit_status = 0;
    entry->state = NOTYET;
    entry->next_tid = 1;

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

    // Scheduler lists
    INSERT(&kernel_state.threads, entry, all_threads);
    INSERT(&parent_pcb->threads, entry, pcb_threads);

    entry->id = get_next_tid(parent_pcb);
    entry->parent = parent_pcb;
    entry->kernel_stack = stack - 1; // leave space for &tcb
    entry->state = NOTYET;

    // Store pointer to tcb at the top of the kernel stack
    *((tcb_t **)stack) = entry;
    
    return entry;
}

/** @brief Creates a new kernel stack for a kernel process
 *
 *  @return Pointer to the top of the new kernel stack
 **/
void *allocate_kernel_stack()
{
    // TODO: Change if lmm region flags are used
    uint32_t mem = (uint32_t)smemalign((size_t)8, PAGE_SIZE);

    return (void *)(mem + PAGE_SIZE - (2*sizeof(int)));
}

