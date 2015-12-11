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
#include <udriv_registry.h>
#include <udriv_kern.h>
#include <user_drivers.h>
#include <asm.h>
#include <scheduler.h>
#include <assert.h>

/** @brief The udriv_send syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_send_syscall(ureg_t state)
{
    struct {
        driv_id_t driv_send;
        message_t msg_send;
        unsigned int msg_size;
    } args;

    tcb_t* tcb = get_tcb();
    ppd_t* ppd = tcb->process->directory;
    if (vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0) {
        goto return_fail;
    }

    // check if driver_id is valid
    devserv_t* server = get_devserv(args.driv_send);
    if ((server == NULL) || (server->driver_id <= UDR_MAX_HW_DEV)) {
        goto return_fail;
    }
    if (args.msg_size > server->bytes) {
        goto return_fail;
    }

    interrupt_t interrupt;
    interrupt.driver_id = server->driver_id;
    interrupt.msg = args.msg_send;
    interrupt.size = args.msg_size;

    // queue interrupt in the device/server buffer
    queue_interrupt(server->owner, interrupt);

    state.eax = 0;
    return;

return_fail:
    state.eax = -1;
    return;
}

/** @brief Wait for an interrupt for the current thread
 *  @param driv_recv Pointer to store driver_id of interrupt
 *  @param msg_recv Pointer to store message received
 *  @param msg_size Pointer to store message size
 *  @return 0 on success, an integer less than 0 on failure
 */
int udriv_wait(tcb_t* tcb, driv_id_t* driv_recv, message_t* msg_recv,
               unsigned int* msg_size)
{
    disable_interrupts();
    // wait for an interrupt if there are none queued
    if (tcb->consumer == tcb->producer) {
        tcb->waiting = 1;
        deschedule(tcb, T_KERN_SUSPENDED);
    }
    enable_interrupts();
    // process the next interrupt
    interrupt_t interrupt = tcb->buffer[tcb->consumer];
    tcb->consumer = next_index_int(tcb->consumer);
    // copy to user pointers
    ppd_t* ppd = tcb->process->directory;
    if (driv_recv != NULL) {
        if (vm_write_locked(ppd, &interrupt.driver_id, (uint32_t)driv_recv,
                            sizeof(driv_id_t)) < 0) {
            return -1;
        }
    }
    if (msg_recv != NULL) {
        if (vm_write_locked(ppd, &interrupt.msg, (uint32_t)msg_recv,
                            sizeof(message_t)) < 0) {
            return -1;
        }
    }
    if (msg_size != NULL) {
        if (vm_write_locked(ppd, &interrupt.size, (uint32_t)msg_size,
                            sizeof(unsigned int)) < 0) {
            return -1;
        }
    }
    return 0;
}

/** @brief Check if the thread is registered to devices with interrupts
 *  @param tcb TCB of thread to check
 *  @return A boolean integer
 */
int has_interrupts(tcb_t* tcb)
{
    // no interrupts if the list of devices/servers registered is empty
    devserv_t* devserv;
    if (Q_IS_EMPTY(&tcb->devserv)) {
        return 0;
    }
    // check if registered devices have interrupts
    Q_FOREACH(devserv, &tcb->devserv, tcb_link)
    {
        // all servers can receive interrupts
        if (devserv->driver_id >= UDR_MAX_HW_DEV) {
            return 1;
        }
        // not all hardware drivers can receive interrupts
        assert(devserv->device_table_entry != NULL);
        if (devserv->device_table_entry->idt_slot != UDR_NO_IDT) {
            return 1;
        }
    }
    return 0;
}

/** @brief The udriv_wait syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_wait_syscall(ureg_t state)
{
    struct {
        driv_id_t* driv_recv;
        message_t* msg_recv;
        unsigned int* msg_size;
    } args;

    tcb_t* tcb = get_tcb();
    ppd_t* ppd = tcb->process->directory;
    if (vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0) {
        goto return_fail;
    }
    // make sure the pointers are writable
    mutex_lock(&ppd->lock);
    if (args.driv_recv != NULL) {
        if (!vm_user_can_write(ppd, args.driv_recv, sizeof(driv_id_t))) {
            goto return_fail_unlock;
        }
    }
    if (args.msg_recv != NULL) {
        if (!vm_user_can_write(ppd, args.msg_recv, sizeof(message_t))) {
            goto return_fail_unlock;
        }
    }
    if (args.msg_size != NULL) {
        if (!vm_user_can_write(ppd, args.msg_size, sizeof(unsigned int))) {
            goto return_fail_unlock;
        }
    }
    mutex_unlock(&ppd->lock);
    // check if thread is registered to any devices/servers with interrupts
    if (!has_interrupts(tcb)) {
        goto return_fail;
    }
    // get an interrupt for the current thread
    if (udriv_wait(tcb, args.driv_recv, args.msg_recv, args.msg_size) < 0) {
        goto return_fail;
    }
    state.eax = 0;
    return;

return_fail_unlock:
    mutex_unlock(&ppd->lock);
return_fail:
    state.eax = -1;
    return;
}


/** @brief The udriv_mmap syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_mmap_syscall(ureg_t state)
{
    lprintf("Thread %d called udriv_mmap. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}
