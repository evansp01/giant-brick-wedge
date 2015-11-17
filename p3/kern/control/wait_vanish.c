#include <control.h>
#include <cond.h>
#include <variable_queue.h>
#include <stdlib.h>
#include <contracts.h>
#include <simics.h>
#include <scheduler.h>

void cleanup_process(pcb_t *pcb)
{
    free_ppd_kernel_mem(&pcb->directory);
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
        lprintf("waiting for child %d", child->id);
        if(child->id < 0){
            MAGIC_BREAK;
        }
        cond_wait(&pcb->wait, &pcb->children_mutex);
        lprintf("finished wait for child %d", child->id);
        child = Q_GET_FRONT(&pcb->children);
    }
    ASSERT(child->state == EXITED);
    int status = child->exit_status;
    int pid = child->id;
    if(status_ptr != NULL){
        ppd_t *ppd = &pcb->directory;
        mutex_lock(&ppd->lock);
        if(vm_write(ppd, &status, status_ptr, sizeof(int)) < 0){
            mutex_unlock(&ppd->lock);
            mutex_unlock(&pcb->children_mutex);
            return -2;
        }
        mutex_unlock(&ppd->lock);
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
    free_ppd_user_mem(&process->directory);
    //now we want to see what our parent has to say
    mutex_lock(&process->parent_mutex);
    //notify all children that you are exited
    pcb_inform_children(process);
    pcb_t* parent = process->parent;
    if (parent == NULL) {
        // nobody is going to wait for you ;(
        lprintf("child %d has no parent", tcb->id);
        return process;
    }
    mutex_lock(&parent->children_mutex);
    process->state = EXITED;
    // Exited people always at front of list for efficient wait
    Q_REMOVE(&parent->children, process, siblings);
    Q_INSERT_FRONT(&parent->children, process, siblings);
    if(parent->waiting > 0){
        parent->waiting--;
        lprintf("child %d is signalling parent %d", tcb->id, parent->id);
        cond_signal(&parent->wait);
    }
    mutex_unlock(&parent->children_mutex);
    mutex_unlock(&process->parent_mutex);
    return NULL;
}

void finalize_exit(tcb_t* tcb)
{   
    if(tcb->process != NULL){
        cleanup_process(tcb->process);
    }
    free_tcb(tcb);
    release_malloc();
}

/** @brief Cleans up and kills a single thread
 *  @param tcb TCB of the thread to kill
 *  @return void
 **/
void vanish_thread(tcb_t *tcb, int failed)
{
    if(failed) {
        alloc_t *alloc;
        Q_FOREACH(alloc, &tcb->process->directory.allocations, list){
            lprintf("alloc start %lx size %lx", alloc->start, alloc->size);
        }
    }
    pcb_t* to_free = thread_exit(tcb, failed);
    acquire_malloc();
    lprintf("thread %d acquired malloc", tcb->id);
    kill_thread(tcb, to_free);
}
