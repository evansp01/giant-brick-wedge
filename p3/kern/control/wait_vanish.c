#include <control.h>
#include <cond.h>
#include <variable_queue.h>
#include <stdlib.h>
#include <contracts.h>
#include <simics.h>
#include <scheduler.h>

void cleanup_process(pcb_t *pcb)
{
    pcb_t *current = get_tcb()->process;
    free_ppd(&pcb->directory, &current->directory);
    free_pcb(pcb);
}

int wait(pcb_t* pcb, int *status_ptr)
{
    mutex_lock(&pcb->children_mutex);
    // aint never gonna get a kid
    if(pcb->waiting == pcb->num_children){
        mutex_unlock(&pcb->children_mutex);
        return -1;
    }
    pcb_t* child = Q_GET_FRONT(&pcb->children);
    if(child->state != EXITED){
        pcb->waiting++;
        cond_wait(&pcb->wait, &pcb->children_mutex);
        child = Q_GET_FRONT(&pcb->children);
    }
    ASSERT(child->state == EXITED);
    int status = child->exit_status;
    int pid = child->id;
    if(status_ptr != NULL){
        ppd_t *ppd = &pcb->directory;
        if(vm_write(ppd, &status, status_ptr, sizeof(int)) < 0){
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
        if(child->state == EXITED){
            // Child is done, we aren't going to wait since we are dying
            cleanup_process(child);
        }
        mutex_unlock(&pcb->children_mutex);
        mutex_unlock(&child->parent_mutex);
    }
}


pcb_t *thread_exit(tcb_t *tcb)
{
    pcb_t* process = tcb->process;
    kernel_remove_thread(tcb);
    int thread_count = pcb_remove_thread(process, tcb);
    // More threads, so we get off easy
    if (thread_count != 0) {
        return NULL;
    }
    //We are cleaning up the last thread
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
    process->state = EXITED;
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

void finalize_exit(tcb_t* tcb)
{
    // we only needed malloc to make sure our thread didn't have it
    release_malloc();
    if(tcb->process != NULL){
        cleanup_process(tcb->process);
    }
    free_tcb(tcb);
}

/** @brief Cleans up and kills a single thread
 *  @param tcb TCB of the thread to kill
 *  @return void
 **/
void vanish_thread(tcb_t *tcb)
{
    pcb_t* to_free = thread_exit(tcb);
    acquire_malloc();
    kill_thread(tcb, to_free);
}
