/** @file syscall.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <debug_print.h>
#include <ureg.h>
#include <stdint.h>
#include <common_kern.h>
#include <control_block.h>
#include <scheduler.h>
#include <stdlib.h>
#include <string.h>
#include <mutex.h>
#include <console.h>
#include <syscall_kern.h>
#include <exec2obj.h>

/** @brief The set_status syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_status_syscall(ureg_t state)
{
    tcb_t *tcb = get_tcb();
    pcb_t *pcb = tcb->process;
    // No mutex needed
    pcb->exit_status = state.esi;
}

/** @brief The task_vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void task_vanish_syscall(ureg_t state)
{
    KPRINTF("Thread %d called task_vanish. Not needed for p3", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The gettid syscall
 *  @param state The current state in user mode
 *  @return void
 */
void gettid_syscall(ureg_t state)
{
    tcb_t* p_tcb = get_tcb();
    // return the tid
    state.eax = p_tcb->id;
}

/** @brief The yield syscall
 *  @param state The current state in user mode
 *  @return void
 */
void yield_syscall(ureg_t state)
{
    int tid = (int)state.esi;
    state.eax = yield(tid);
}


/** @brief The deschedule syscall
 *  @param state The current state in user mode
 *  @return void
 */
void deschedule_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    state.eax = user_deschedule(tcb, state.esi);
}

/** @brief The make_runnable syscall
 *  @param state The current state in user mode
 *  @return void
 */
void make_runnable_syscall(ureg_t state)
{
    mutex_lock(&kernel_state.threads_mutex);
    tcb_t *make_runnable = get_tcb_by_id(state.esi);
    if(make_runnable == NULL){
        mutex_unlock(&kernel_state.threads_mutex);
        state.eax = -1;
    } else {
        state.eax = user_schedule(make_runnable, &kernel_state.threads_mutex);
    }
}

/** @brief The get_ticks syscall
 *  @param state The current state in user mode
 *  @return void
 */
void get_ticks_syscall(ureg_t state)
{
    // don't need locks, just read whatever is there
    state.eax = get_ticks();
}

/** @brief The sleep syscall
 *  @param state The current state in user mode
 *  @return void
 */
void sleep_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    int status = add_sleeper(tcb, state.esi);
    if(status < 0){
        state.eax = status;
        return;
    }
    if(status > 0){
        release_sleeper(tcb);
    }
    state.eax = 0;
}

/** @brief The new_pages syscall
 *  @param state The current state in user mode
 *  @return void
 */
void new_pages_syscall(ureg_t state)
{
    struct {
        uint32_t start;
        uint32_t size;
    } args;

    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;
    mutex_lock(&ppd->lock);
    if(vm_read(ppd, &args, (void *)state.esi, sizeof(args)) < 0){
        goto return_fail;
    }
    if(page_align(args.start) != args.start || args.size % PAGE_SIZE != 0){
        goto return_fail;
    }
    if(!vm_user_can_alloc(ppd, (void *)args.start, args.size)){
        DPRINTF("Space not allocable");
        goto return_fail;
    }
    if(vm_alloc_readwrite(ppd, (void *)args.start, args.size) < 0){
        goto return_fail;
    }
    mutex_unlock(&ppd->lock);
    state.eax = 0;
    return;

return_fail:
    mutex_unlock(&ppd->lock);
    state.eax = -1;
    return;
}

/** @brief The remove_pages syscall
 *  @param state The current state in user mode
 *  @return void
 */
void remove_pages_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;
    mutex_lock(&ppd->lock);
    int result = vm_free(ppd, (void*)state.esi);
    mutex_unlock(&ppd->lock);
    state.eax = result;
}

/** @brief The halt syscall
 *  @param state The current state in user mode
 *  @return void
 */
void halt_syscall(ureg_t state)
{
    // Halt machines running on simics
    sim_halt();
    // Halt machines running on real hardware
    halt_asm();
    // Hmmm, things didn't go so well
    panic("We can't be killed!");
}

/** @brief The readfile syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readfile_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;
    struct {
        char *filename;
        char *buf;
        int count;
        int offset;
    } args;
    mutex_lock(&ppd->lock);
    if (vm_read(ppd, &args, (void*)state.esi, sizeof(args)) < 0) {
        goto return_fail;
    }
    // make sure the filename is readable and short enough
    int strlen = vm_user_strlen(ppd, args.filename, MAX_EXECNAME_LEN);
    if(strlen > MAX_EXECNAME_LEN || strlen < 0){
        goto return_fail;
    }
    // make sure the buffer is user writeable
    if(!vm_user_can_write(ppd, args.buf, args.count)){
        goto return_fail;
    }
    // back the buffer with real frames to prevent pagefaults
    if(vm_back(ppd, (uint32_t)args.buf, args.count) < 0){
        goto return_fail;
    }
    // read from the file
    int bytes = getbytes(args.filename, args.offset, args.count, args.buf);
    mutex_unlock(&ppd->lock);
    state.eax = bytes;
    return;

return_fail:
    state.eax = -1;
    mutex_unlock(&ppd->lock);
    return;

}

/** @brief The misbehave syscall
 *  @param state The current state in user mode
 *  @return void
 */
void misbehave_syscall(ureg_t state)
{
    DPRINTF("Our kernel is probably already misbehaving.");
}
