/** @file syscall.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <simics.h>
#include <ureg.h>
#include <stdint.h>
#include <common_kern.h>
#include <control.h>
#include <loader.h>
#include <scheduler.h>
#include <stdlib.h>
#include <string.h>
#include <asm.h>
#include <sem.h>
#include <devices.h>
#include <console.h>
#include <video_defines.h>

// TODO: Decide on arbitrary max length
#define MAX_LEN 1024
#define INVALID_COLOR 0x90
sem_t read_sem;
sem_t print_sem;

/** @brief Initializes the semaphores used in the console syscalls
 *  @return void
 */
void init_syscall_sem()
{
    sem_init(&read_sem, 1);
    sem_init(&print_sem, 1);
}

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

/** @brief The vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void vanish_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    kill_thread(tcb);
}

/** @brief The task_vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void task_vanish_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called task_vanish. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
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
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called yield. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The deschedule syscall
 *  @param state The current state in user mode
 *  @return void
 */
void deschedule_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    state.eax = deschedule(tcb, state.esi);
}

/** @brief The make_runnable syscall
 *  @param state The current state in user mode
 *  @return void
 */
void make_runnable_syscall(ureg_t state)
{
    tcb_t *make_runnable = get_tcb_by_id(state.esi);
    if(make_runnable == NULL){
        state.eax = -1;
    } else {
        state.eax = schedule(make_runnable);
    }
}

/** @brief The get_ticks syscall
 *  @param state The current state in user mode
 *  @return void
 */
void get_ticks_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called get_ticks. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The sleep syscall
 *  @param state The current state in user mode
 *  @return void
 */
void sleep_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called sleep. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The new_pages syscall
 *  @param state The current state in user mode
 *  @return void
 */
void new_pages_syscall(ureg_t state)
{
    struct {
        void *start;
        uint32_t size;
    } packet;

    tcb_t* tcb = get_tcb();
    ppd_t *ppd = &tcb->process->directory;
    mutex_lock(&ppd->lock);
    if(vm_read(ppd, &packet, (void *)state.esi, sizeof(packet)) < 0){
        goto return_fail;
    }
    if(!vm_user_can_alloc(ppd, packet.start, packet.size)){
        goto return_fail;
    }
    if(vm_alloc_readwrite(ppd, packet.start, packet.size) < 0){
        goto return_fail;
    }
    mutex_unlock(&ppd->lock);
    state.eax = 0;
    return;

return_fail:
    lprintf("New pages failed %lx %lx", (uint32_t)packet.start, packet.size);
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
    ppd_t *ppd = &tcb->process->directory;
    mutex_lock(&ppd->lock);
    int result = vm_free(ppd, (void*)state.esi);
    mutex_unlock(&ppd->lock);
    state.eax = result;
}

/** @brief The readline syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readline_syscall(ureg_t state)
{
    typedef struct args {
        int len;
        char *buf;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    
    /*
    // Error: buf is not a valid memory address
    if (vm_user_can_write(ppd_t* ppd, void* start, uint32_t size)) {
        state.eax = -1;
        return;
    }
    */
    
    // Error: len is unreasonably large
    if (arg->len > MAX_LEN) {
        state.eax = -3;
        return;
    }
    
    sem_wait(&read_sem);
    char c;
    int i = 0;
    char temp[arg->len];
    while (i < arg->len) {
        if ((c = readchar()) == -1) {
            continue;
        }
        putbyte(c);
        
        // Backspace character
        if (c == '\b') {
            i--;
        }
        // Standard character
        else {
            temp[i] = c;
            i++;
            if (c == '\n')
                break;
        }
    }
    sem_signal(&read_sem);
    
    memcpy(arg->buf, temp, i);
    state.eax = i;
}

/** @brief The getchar syscall
 *  @param state The current state in user mode
 *  @return void
 */
void getchar_syscall(ureg_t state)
{
    char c;
    sem_wait(&read_sem);
    while ((c = readchar()) == -1) {
        continue;
    }
    sem_signal(&read_sem);
    state.eax = c;
}

/** @brief The print syscall
 *  @param state The current state in user mode
 *  @return void
 */
void print_syscall(ureg_t state)
{
    typedef struct args {
        int len;
        char *buf;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    
    /*
    // Error: buf is not a valid memory address
    if (??) {
        state.eax = -1;
        return;
    }
    
    // Error: len is unreasonably large
    if (arg->len > MAX_LEN) {
        state.eax = -2;
        return;
    }
    */
    
    sem_wait(&print_sem);
    putbytes(arg->buf, arg->len);
    sem_signal(&print_sem);
    state.eax = 0;
}

/** @brief The set_term_color syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_term_color_syscall(ureg_t state)
{
    int color = (int)state.esi;
    
    // Error: color is not valid
    if (color >= INVALID_COLOR) {
        state.eax = -1;
        return;
    }
    
    set_term_color(color);
    state.eax = 0;
}

/** @brief The set_cursor_pos syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_cursor_pos_syscall(ureg_t state)
{
    typedef struct args {
        int row;
        int col;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    
    state.eax = set_cursor(arg->row, arg->col);
}

/** @brief The get_cursor_pos syscall
 *  @param state The current state in user mode
 *  @return void
 */
void get_cursor_pos_syscall(ureg_t state)
{
    typedef struct args {
        int *row;
        int *col;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    
    // Error: arguments are invalid
    // TODO: Check if pointers are on user-writable memory?
    if ((arg->row == NULL)||(arg->col == NULL)) {
        state.eax = -1;
        return;
    }
    
    get_cursor(arg->row, arg->col);
    state.eax = 0;
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
    panic("Running on real hardware, please add code to initiate shutdown\
           based on current hardware implementation!");
}


/** @brief The readfile syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readfile_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called readfile. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The misbehave syscall
 *  @param state The current state in user mode
 *  @return void
 */
void misbehave_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called misbehave. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The swexn syscall
 *  @param state The current state in user mode
 *  @return void
 */
void swexn_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called swexn. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}
