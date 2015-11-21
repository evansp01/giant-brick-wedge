#include <control_block.h>
#include <cond.h>
#include <variable_queue.h>
#include <stdlib.h>
#include <contracts.h>
#include <simics.h>
#include <scheduler.h>
#include <malloc_wrappers.h>

/** @brief The vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void vanish_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    vanish_thread(tcb, THREAD_EXIT_SUCCESS);
}

/** @brief The wait syscall
 *  @param state The current state in user mode
 *  @return void
 */
void wait_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    state.eax = wait(tcb->process, (int*)state.esi);
}

/** @brief Waits on children of a process if any children exist
 *
 *  @param pcb The process whose children should be waited on
 *  @param status_ptr Will be set to the exit status of children if not NULL
 *  @return Zero if a child was waited on, less than zero on error or no child
 **/
int wait(pcb_t* pcb, int *status_ptr)
{
    mutex_lock(&pcb->children_mutex);
    // aint never gonna get a kid
    if(pcb->waiting == pcb->num_children){
        mutex_unlock(&pcb->children_mutex);
        return -1;
    }
    pcb_t* child = Q_GET_FRONT(&pcb->children);
    if(child->state != P_EXITED){
        pcb->waiting++;
        assert(child->id > 0);
        cond_wait(&pcb->wait, &pcb->children_mutex);
        child = Q_GET_FRONT(&pcb->children);
    }
    assert(child->state == P_EXITED);
    int status = child->exit_status;
    int pid = child->id;
    if(status_ptr != NULL){
        ppd_t *ppd = pcb->directory;
        if(vm_write_locked(ppd, &status,(uint32_t)status_ptr,
                           sizeof(int)) < 0){
            mutex_unlock(&pcb->children_mutex);
            return -2;
        }
    }
    Q_REMOVE(&pcb->children, child, siblings);
    pcb->num_children--;
    free_pcb(child);
    mutex_unlock(&pcb->children_mutex);
    return pid;
}

/** @brief Informs all children of a process that the process is exiting so
 *         that they know to clean themselves up
 *
 *  @param pcb The process which is exiting
 *  @return void
 **/
void pcb_inform_children(pcb_t* pcb)
{
    pcb_t* child;
    pcb_t* tmp;
    assert(pcb != kernel_state.init->process);
    // We don't need this pcb's mutex since there are no remaining
    // processes which can
    Q_FOREACH_SAFE(child, tmp, &pcb->children, siblings)
    {
        assert(child != kernel_state.init->process);
        mutex_lock(&child->parent_mutex);
        mutex_lock(&pcb->children_mutex);
        Q_REMOVE(&pcb->children, child, siblings);
        pcb->num_children--;
        pcb_add_child(kernel_state.init->process, child);
        mutex_unlock(&pcb->children_mutex);
        mutex_unlock(&child->parent_mutex);
    }
}

/** @brief Cleans up a thread, and it's process if it is the last thread
 *
 *  @param tcb The thread to clean up
 *  @param failed Is this thread being killed because it failed
 *  @return pcb The process pcb if it needs to be freed, NULL otherwise
 **/
ppd_t *thread_exit(tcb_t *tcb, thread_exit_state_t failed)
{
    pcb_t* process = tcb->process;
    kernel_remove_thread(tcb);
    int thread_count = pcb_remove_thread(process, tcb);
    // More threads, so we get off easy
    if (thread_count != 0) {
        return NULL;
    }
    if(failed == THREAD_EXIT_FAILED){
        process->exit_status = -2;
    }
    //We are cleaning up the last thread
    //first deallocate the user memory, we don't need it
    free_ppd_user_mem(process->directory);
    //now we want to see what our parent has to say
    mutex_lock(&process->parent_mutex);
    //notify all children that you are exited
    pcb_inform_children(process);
    //neither idle nor init are allowed to exit
    //and everyone else must have a parent
    pcb_t* parent = process->parent;
    assert(parent != NULL);
    mutex_lock(&parent->children_mutex);
    process->state = P_EXITED;
    // Exited people always at front of list for efficient wait
    Q_REMOVE(&parent->children, process, siblings);
    Q_INSERT_FRONT(&parent->children, process, siblings);
    if (parent->waiting > 0) {
        parent->waiting--;
        mutex_unlock(&process->parent_mutex);
        cond_signal(&parent->wait);
        mutex_unlock(&parent->children_mutex);
    } else {
        mutex_unlock(&parent->children_mutex);
        mutex_unlock(&process->parent_mutex);
    }
    return process->directory;
}

/** @brief Frees memory associated with a thread and process which cannot
 *         be freed conveniently by the thread
 *
 *  @param tcb The thread to free
 *  @return void
 **/
void finalize_exit(tcb_t* tcb)
{
    if(tcb->free_pointer != NULL){
        _free_ppd_kernel_mem(tcb->free_pointer);
    }
    _free_tcb(tcb);
}

/** @brief Cleans up a deschedules a thread
 *  @param tcb The thread to kill
 *  @param failed Is this thread being killed because it failed?
 *  @return void
 **/
void vanish_thread(tcb_t *tcb, thread_exit_state_t failed)
{
    tcb->free_pointer = thread_exit(tcb, failed);
    acquire_malloc();
    free_later(tcb);
    kill_thread(tcb);
}
