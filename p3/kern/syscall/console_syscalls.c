/** @file console_syscalls.c
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
#include <control_block.h>
#include <scheduler.h>
#include <stdlib.h>
#include <string.h>
#include <mutex.h>
#include <console.h>
#include <syscall_kern.h>
#include <exec2obj.h>


static mutex_t print_mutex;

/** @brief Initializes the mutexes used in the console syscalls
 *  @return void
 */
void init_print()
{
    mutex_init(&print_mutex);
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
}

/** @brief The print syscall
 *  @param state The current state in user mode
 *  @return void
 */
void print_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;
    struct {
        int len;
        char *buf;
    } args;

    if(vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0){
        state.eax = -1;
        return;
    }
    // printing zero characters is easy
    if(args.len == 0){
        state.eax = 0;
        return;
    }
    // Error: len is unreasonable
    if (args.len < 0) {
        state.eax = -1;
        return;
    }
    mutex_lock(&ppd->lock);
    // Error: buf is not a valid memory address
    if (!vm_user_can_read(ppd, (void *)args.buf, args.len)) {
        state.eax = -2;
        return;
    }
    mutex_lock(&print_mutex);
    putbytes(args.buf, args.len);
    mutex_unlock(&print_mutex);
    mutex_unlock(&ppd->lock);
    state.eax = 0;
}

/** @brief The set_term_color syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_term_color_syscall(ureg_t state)
{
    int color = (int)state.esi;
    state.eax = set_term_color(color);
}

/** @brief The set_cursor_pos syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_cursor_pos_syscall(ureg_t state)
{
    ppd_t *ppd = get_tcb()->process->directory;
    struct {
        int row;
        int col;
    } args;
    if(vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0){
        state.eax = -1;
        return;
    }
    state.eax = set_cursor(args.row, args.col);
}

/** @brief The get_cursor_pos syscall
 *  @param state The current state in user mode
 *  @return void
 */
void get_cursor_pos_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;
    struct {
        uint32_t row;
        uint32_t col;
    } args;
    if(vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0){
        state.eax = -1;
        return;
    }
    // get the row and column
    int row, col;
    get_cursor(&row, &col);
    if(vm_write_locked(ppd, &row, args.row, sizeof(int)) < 0 ||
       vm_write_locked(ppd, &col, args.col, sizeof(int)) < 0) {
        state.eax = -1;
        return;
    }
    state.eax = 0;
}

