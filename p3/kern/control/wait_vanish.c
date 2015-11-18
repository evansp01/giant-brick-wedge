#include <control.h>
#include <cond.h>
#include <variable_queue.h>
#include <stdlib.h>
#include <contracts.h>
#include <simics.h>
#include <scheduler.h>

/** @brief Frees all kernel memory associated with a process without locks
 *  @param pcb The process to free
 *  @return void
 **/
void _cleanup_process(pcb_t *pcb)
{
    _free_ppd_kernel_mem(pcb->directory);
    _free_pcb(pcb);
}

/** @brief Frees all kernel memory associated with a process
 *  @param pcb The process to free
 *  @return void
 **/
void cleanup_process(pcb_t *pcb)
{
    acquire_malloc();
    _cleanup_process(pcb);
    release_malloc();
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
        if(child->id < 0){
            MAGIC_BREAK;
        }
        cond_wait(&pcb->wait, &pcb->children_mutex);
        child = Q_GET_FRONT(&pcb->children);
    }
    ASSERT(child->state == EXITED);
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
    cleanup_process(child);
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
    // We don't need this pcb's mutex since there are no remaining
    // processes which can
    Q_FOREACH_SAFE(child, tmp, &pcb->children, siblings)
    {
        mutex_lock(&child->parent_mutex);
        mutex_lock(&pcb->children_mutex);
        Q_REMOVE(&pcb->children, child, siblings);
        pcb->num_children--;
        child->parent = NULL;
        if(child->state == P_EXITED){
            // Child is done, we aren't going to wait since we are dying
            cleanup_process(child);
        }
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
pcb_t *thread_exit(tcb_t *tcb, int failed)
{
    pcb_t* process = tcb->process;
    kernel_remove_thread(tcb);
    int thread_count = pcb_remove_thread(process, tcb);
    // More threads, so we get off easy
    if (thread_count != 0) {
        return NULL;
    }
    if(failed){
        process->exit_status = -2;
    }
    //We are cleaning up the last thread
    //first deallocate the user memory, we don't need it
    free_ppd_user_mem(process->directory);
    //now we want to see what our parent has to say
    mutex_lock(&process->parent_mutex);
    //notify all children that you are exited
    pcb_inform_children(process);
    pcb_t* parent = process->parent;
    if (parent == NULL) {
        // nobody is going to wait for you ;(
        return process;
    }
    mutex_lock(&parent->children_mutex);
    process->state = P_EXITED;
    // Exited people always at front of list for efficient wait
    Q_REMOVE(&parent->children, process, siblings);
    Q_INSERT_FRONT(&parent->children, process, siblings);
    if(parent->waiting > 0){
        parent->waiting--;
        cond_signal(&parent->wait);
    }
    mutex_unlock(&parent->children_mutex);
    mutex_unlock(&process->parent_mutex);
    return NULL;
}

/** @brief Frees memory associated with a thread and process which cannot
 *         be freed conveniently by the thread
 *
 *  @param The thread to free
 *  @return void
 **/
void finalize_exit(tcb_t* tcb)
{
    if(tcb->process != NULL){
        _cleanup_process(tcb->process);
    }
    _free_tcb(tcb);
    release_malloc();
}

/** @brief Cleans up a deschedules a thread
 *  @param tcb The thread to kill
 *  @param failed Is this thread being killed because it failed?
 *  @return void
 **/
void vanish_thread(tcb_t *tcb, int failed)
{
    pcb_t* to_free = thread_exit(tcb, failed);
    acquire_malloc();
    kill_thread(tcb, to_free);
}
