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
#include <common.h>
#include <console.h>
#include <video_defines.h>
#include <seg.h>
#include <eflags.h>
#include <syscall_kern.h>

#define MAX_LEN (CONSOLE_WIDTH*(CONSOLE_HEIGHT-1))
#define INVALID_COLOR 0x90
#define USER_FLAGS (EFL_RF|EFL_OF|EFL_DF|EFL_SF|EFL_ZF|EFL_AF|EFL_PF|EFL_CF)

/** @brief Struct for variables required for syscalls */
typedef struct {
    sem_t read_sem;
    sem_t print_sem;
} sysvars_t;

static sysvars_t sysvars;

/** @brief Initializes the semaphores used in the console syscalls
 *  @return void
 */
void init_syscalls()
{
    sem_init(&sysvars.read_sem, 1);
    sem_init(&sysvars.print_sem, 1);
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
    vanish_thread(tcb);
}

/** @brief The task_vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void task_vanish_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called task_vanish. Not needed for p3", tcb->id);
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
    uint32_t slept_for = add_sleeper(tcb, state.esi);
    if(slept_for < 0){
        state.eax = slept_for;
        return;
    }
    if(slept_for > 0){
        release_sleeper(slept_for);
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
        lprintf("Space no allocable");
        goto return_fail;
    }
    if(vm_alloc_readwrite(ppd, packet.start, packet.size) < 0){
        goto return_fail;
    }
    mutex_unlock(&ppd->lock);
    lprintf("New pages suceeded %lx %lx", (uint32_t)packet.start, packet.size);
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
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = &tcb->process->directory;
    typedef struct args {
        int len;
        char *buf;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    
    // Error: len is unreasonable
    if ((arg->len > MAX_LEN)||(arg->len <= 0)) {
        state.eax = -1;
        return;
    }
    // Error: buf is not a valid memory address
    if (!vm_user_can_write(ppd, (void *)arg->buf, arg->len)) {
        state.eax = -2;
        return;
    }
    
    sem_wait(&sysvars.read_sem);
    int num_bytes = readline(arg->len, arg->buf, tcb);
    sem_signal(&sysvars.read_sem);
    
    state.eax = num_bytes;
}

/** @brief The getchar syscall
 *  @param state The current state in user mode
 *  @return void
 */
void getchar_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called getchar. Not needed for p3", tcb->id);
    while(1) {
        continue;
    }
    /*
    char c;
    sem_wait(&read_sem);
    while ((c = readchar()) == -1) {
        continue;
    }
    sem_signal(&read_sem);
    state.eax = c;
    */
}

/** @brief The print syscall
 *  @param state The current state in user mode
 *  @return void
 */
void print_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = &tcb->process->directory;
    typedef struct args {
        int len;
        char *buf;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    
    // Error: len is unreasonable
    if ((arg->len > MAX_LEN)||(arg->len <= 0)) {
        state.eax = -1;
        return;
    }
    // Error: buf is not a valid memory address
    if (!vm_user_can_read(ppd, (void *)arg->buf, arg->len)) {
        state.eax = -2;
        return;
    }
    
    sem_wait(&sysvars.print_sem);
    putbytes(arg->buf, arg->len);
    sem_signal(&sysvars.print_sem);
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
    halt_asm();
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

/** @brief Checks the user provided arguments for swexn
 *  @param tcb TCB of current thread
 *  @param eip Address of first instruction of user exception handler
 *  @param esp User exception stack
 *  @param regs User provided return register state
 *  @return 0 on success, a negative integer on failure
 */
int check_swexn(tcb_t *tcb, swexn_handler_t eip, void *esp, ureg_t *regs,
                uint32_t eflags)
{
    ppd_t *ppd = &tcb->process->directory;
    
    // Error: Provided eip and/or esp3 are unreasonable
    if ((eip != 0)&&(esp != 0)) {
        // Check that eip is user read memory
        if (!vm_user_can_read(ppd, eip, sizeof(void*))) {
            return -1;
        }
        // Check that esp3 is in user write memory
        if (!vm_user_can_write(ppd, esp, sizeof(void*))) {
            return -2;
        }
    }
    
    // Error: Provided register values are unreasonable
    if (regs != NULL) {
        // Check that newureg is in user memory
        if (!vm_user_can_read(ppd, regs, sizeof(void*))) {
            return -3;
        }
        // Check ds/es/fs/gs/ss/cs
        if ((regs->ds != SEGSEL_USER_DS)||(regs->es != SEGSEL_USER_DS)||
            (regs->fs != SEGSEL_USER_DS)||(regs->gs != SEGSEL_USER_DS)||
            (regs->ss != SEGSEL_USER_DS)||(regs->cs != SEGSEL_USER_CS)) {
            return -4;
        }
        // Check that eip is in user read memory
        if (!vm_user_can_read(ppd, (void *)regs->eip, sizeof(void*))) {
            return -5;
        }
        // Check eflags
        if ((eflags&(~USER_FLAGS)&(~EFL_RF)) != (regs->eflags&(~USER_FLAGS))) {
            return -6;
        }
        // Check that esp is in user write memory
        if (!vm_user_can_write(ppd, (void *)regs->esp, sizeof(void*))) {
            return -7;
        }
    }
    return 0;
}

/** @brief The swexn syscall
 *  @param state The current state in user mode
 *  @return void
 */
void swexn_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    
    typedef struct args {
        void *esp3;
        swexn_handler_t eip;
        void *arg;
        ureg_t *newureg;
    } args_t;
    args_t *arg = (args_t *)state.esi;
    int ret = check_swexn(tcb, arg->eip, arg->esp3, arg->newureg, state.eflags);
    // If either request cannot be carried out, syscall must fail
    if (ret < 0) {
        state.eax = -1;
        return;
    }
    
    // Deregister handler if one is registered
    if ((arg->esp3 == 0)||(arg->eip == 0)) {
        deregister_swexn(tcb);
    }
    
    // Register a new software exception handler
    else {
        uint32_t stack = (uint32_t)arg->esp3 - sizeof(void *);
        register_swexn(tcb, arg->eip, arg->arg, (void *)stack);
    }
    
    // Adopt specific register values
    if (arg->newureg != NULL) {
        state = *(arg->newureg);
    }
    else {
        state.eax = 0;
    }
}
