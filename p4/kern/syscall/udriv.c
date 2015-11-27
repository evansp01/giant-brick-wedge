/** @file udriv.c
 *
 *  @brief Functions to handle udriv syscalls
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <debug_print.h>
#include <control_block.h>
 
/** @brief The udriv_register syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_register_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_register. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The udriv_deregister syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_deregister_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_deregister. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The udriv_send syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_send_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_send. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The udriv_wait syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_wait_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_wait. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The udriv_inb syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_inb_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_inb. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The udriv_outb syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_outb_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_outb. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}

/** @brief The udriv_mmap syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_mmap_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_mmap. Not yet implemented.", get_tcb()->id);
    while(1) {
        continue;
    }
}