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

// Global kernel state with process and thread info
kernel_state_t kernel_state;

/** @brief Initializes the global lists of processes and threads
 *  @return void
 **/
void init_kernel_state()
{
    INIT_STRUCT(&kernel_state.processes);
    INIT_STRUCT(&kernel_state.threads);
    kernel_state.next_pid = 1;
}

/** @brief Gives the next available process/thread id number
 *  @return Next sequential process id
 **/
int get_next_pid()
{
    int id = kernel_state.next_pid;
    kernel_state.next_pid++;
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
    tcb_t *entry = (tcb_t *)smalloc(sizeof(tcb_t));

    Q_INIT_ELEM(entry, all_threads);
    Q_INIT_ELEM(entry, pcb_threads);

    // Scheduler lists
    INSERT(&kernel_state.threads, entry, all_threads);
    INSERT(&parent_pcb->threads, entry, pcb_threads);

    entry->id = get_next_pid();
    entry->parent = parent_pcb;
    entry->kernel_stack = stack;
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
    uint32_t mem = (uint32_t)smemalign(PAGE_SIZE, PAGE_SIZE);
    return (void *)(mem + PAGE_SIZE - (2*sizeof(int)));
}

/** @brief Gets the tcb from the top of the kernel stack
 *
 *  @param An address on the current kernel stack
 *  @return Pointer to the tcb for the current kernel thread
 **/
tcb_t *get_tcb_from_addr(void *addr)
{
    uint32_t tcb_addr = (((uint32_t)addr)&0xFFFFF000)|0xFF8;
    return *(tcb_t **)tcb_addr;
}
