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

/** @brief Can a device access a give port
 *  @param port_region The permissions of the device
 *  @param port The port
 *  @return a boolean integer
 **/
int can_access(const udrv_region_t* port_region, unsigned int port)
{
    unsigned int base = port_region->base;
    unsigned int len = port_region->len;
    if (base <= port && port <= base + len) {
        return 1;
    }
    return 0;
}

/** @brief Check if the arguments for registering a hardware device are valid
 *  @param device Pointer to device entry
 *  @param in_port Port address if bytes are required
 *  @param in_bytes Number of bytes requested from port
 *  @return 0 on success, an integer less than 0 on failure
 */
int claim_hw_drv(tcb_t* tcb, devserv_t* device, unsigned int in_port,
                 unsigned int in_bytes)
{
    // in_bytes must be 0 or 1 for a hardware device
    if (in_bytes > 1) {
        return -1;
    }
    // bytes are to be requested from port during interrupt
    if (in_bytes == 1) {
        const dev_spec_t* driv = device->device_table_entry;
        assert(driv != NULL); // all device tables entries should have this
        int i;
        for (i = 0; i < driv->port_regions_cnt; i++) {
            const udrv_region_t* port_region = &driv->port_regions[i];
            // in_port is valid for the device
            if (can_access(port_region, in_port)) {
                break;
            }
        }
        if (i == driv->port_regions_cnt) {
            return -1;
        }
    }
    mutex_lock(&device->mutex);
    if (device->owner != NULL) {
        if (device->owner != tcb) {
            // someone beat us to ownership
            mutex_unlock(&device->mutex);
            return -1;
        } else {
            mutex_unlock(&device->mutex);
        }
    } else {
        // lock in the current thread as the owner
        device->owner = tcb;
        mutex_unlock(&device->mutex);
    }
    return 0;
}

/** @brief Register the hardware driver
 *  @param device Pointer to device entry
 *  @param tcb TCB of current thread
 *  @param in_port Port to read, if any
 *  @param in_bytes Number of bytes to read, if any
 *  @return void
 */
void register_hw_drv(devserv_t* device, tcb_t* tcb, unsigned int in_port,
                     unsigned int in_bytes)
{
    // set request for bytes from port
    if (in_bytes == 1) {
        device->port = in_port;
        device->bytes = in_bytes;
    }
    // add device to current thread's tcb
    Q_INSERT_FRONT(&tcb->devserv, device, tcb_link);
}

devserv_t* create_and_register_devserv(int driver_id, tcb_t* owner)
{
    devserv_t* server = create_devserv_entry(driver_id);
    server->owner = owner;
    if (check_add_devserv(server) < 0) {
        free_devserv_entry(server);
        return NULL;
    }
    return server;
}

/** @brief Check if the arguments for registering a server are valid
 *  Creates the server entry if server does not yet exist
 *  Take ownership of the server entry before returning
 *  
 *  @param tcb TCB of the potential owner
 *  @param driver_id ID of requested driver
 *  @param in_bytes Number of bytes requested
 *  @return 0 on success, an integer less than 0 on failure
 */
int valid_server(tcb_t* tcb, driv_id_t driver_id, unsigned int in_bytes)
{
    // driver id must valid
    if ((driver_id <= UDR_MAX_HW_DEV) || (driver_id >= UDR_MIN_ASSIGNMENT)) {
        return -1;
    }
    // in_bytes must be between 0 and sizeof(message_t)
    if (in_bytes > sizeof(message_t)) {
        return -1;
    }
    // server must not already be owned
    if (driver_id != UDR_ASSIGN_REQUEST) {
        devserv_t* server = get_devserv(driver_id);
        // server has not been created
        if (server == NULL) {
            server = create_and_register_devserv(driver_id, tcb);
            if (server == NULL)
                return -1;
        }
        // server already exists
        else {
            // prevent concurrency issues with ownership
            mutex_lock(&server->mutex);
            if (server->owner != NULL) {
                // someone beat us to ownership
                mutex_unlock(&server->mutex);
                return -1;
            } else {
                // lock in the current thread as the owner
                server->owner = tcb;
                mutex_unlock(&server->mutex);
            }
        }
    }
    return 0;
}

/** @brief Register the server
 *  @param server Pointer to device entry
 *  @param tcb TCB of current thread
 *  @param in_bytes Number of bytes to read, if any
 *  @return void
 */
void register_server(devserv_t* server, tcb_t* tcb, unsigned int in_bytes)
{
    // set request for bytes
    if (in_bytes > 0) {
        server->bytes = in_bytes;
    }
    // add server to current thread's tcb
    Q_INSERT_FRONT(&tcb->devserv, server, tcb_link);
}

/** @brief The udriv_register syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_register_syscall(ureg_t state)
{
    struct {
        driv_id_t driver_id;
        unsigned int in_port;
        unsigned int in_bytes;
    } args;

    tcb_t* tcb = get_tcb();
    ppd_t* ppd = tcb->process->directory;
    if (vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0) {
        state.eax = -1;
        return;
    }

    // driver_id refers to a hardware device
    if (args.driver_id < UDR_MAX_HW_DEV) {
        devserv_t* device = get_devserv(args.driver_id);
        // device not in the device table
        if (device == NULL) {
            state.eax = -1;
        } else if (claim_hw_drv(tcb, device, args.in_port, args.in_bytes) < 0) {
            state.eax = -1;
        } else {
            register_hw_drv(device, tcb, args.in_port, args.in_bytes);
            state.eax = device->driver_id;
        }
        return;
    }

    // driver_id refers to a user server
    // note: in_port is ignored for servers
    // check argument validity
    if (valid_server(tcb, args.driver_id, args.in_bytes) < 0) {
        state.eax = -1;
        return;
    }
    devserv_t* server;
    // user has requested to be assigned a server id
    if (args.driver_id == UDR_ASSIGN_REQUEST) {
        driv_id_t assigned_id = assign_driver_id();
        server = create_and_register_devserv(assigned_id, tcb);
    } else {
        // driver_id is in list of defined servers
        server = get_devserv(args.driver_id);
    }
    register_server(server, tcb, args.in_bytes);
    state.eax = server->driver_id;
}

/** @brief The udriv_deregister syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_deregister_syscall(ureg_t state)
{
    driv_id_t driver_id = state.esi;
    tcb_t* tcb = get_tcb();
    // check if driver_id is valid
    devserv_t* devserv = get_devserv(driver_id);
    if (devserv == NULL) {
        return;
    }
    // check if calling thread is registered to driver_id
    mutex_lock(&devserv->mutex);
    if (devserv->owner != tcb) {
        mutex_unlock(&devserv->mutex);
        return;
    }
    // get rid of ownership
    devserv->owner = NULL;
    devserv->bytes = 0;
    devserv->port = 0;
    Q_REMOVE(&tcb->devserv, devserv, tcb_link);
    mutex_unlock(&devserv->mutex);
    // if it is a kernel assigned server id, clean up the data structures
    if (devserv->driver_id > UDR_MIN_ASSIGNMENT) {
        remove_devserv(devserv);
        free_devserv_entry(devserv);
    }
}

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

/** @brief Checks if the thread has permissions to access the port
 *  @param tcb TCB of the current thread
 *  @param port Port that needs to be validated
 *  @return 0 if the thread has permissions, an integer less than 0 if not
 */
int check_port_permissions(tcb_t* tcb, unsigned int port)
{
    devserv_t* devserv;
    Q_FOREACH(devserv, &tcb->devserv, tcb_link)
    {
        // only hardware drivers can access ports
        if (devserv->driver_id >= UDR_MAX_HW_DEV) {
            continue;
        }
        const dev_spec_t* dev = devserv->device_table_entry;
        assert(dev != NULL); // all device tables entries should have this
        int i;
        for (i = 0; i < dev->port_regions_cnt; i++) {
            const udrv_region_t* port_region = &dev->port_regions[i];
            // in_port is valid for the device
            if (can_access(port_region, port)) {
                return 0;
            }
        }
    }
    return -1;
}

/** @brief The udriv_inb syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_inb_syscall(ureg_t state)
{
    struct {
        unsigned int port;
        unsigned char* val;
    } args;

    tcb_t* tcb = get_tcb();
    ppd_t* ppd = tcb->process->directory;
    if (vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0) {
        goto return_fail;
    }
    mutex_lock(&ppd->lock);
    if (args.val != NULL) {
        if (!vm_user_can_write(ppd, args.val, sizeof(unsigned char))) {
            mutex_unlock(&ppd->lock);
            goto return_fail;
        }
    }
    mutex_unlock(&ppd->lock);
    if (check_port_permissions(tcb, args.port) < 0) {
        goto return_fail;
    }

    uint8_t in = inb(args.port);
    if (args.val != NULL) {
        if (vm_write_locked(ppd, &in, (uint32_t)args.val,
                            sizeof(unsigned char)) < 0) {
            goto return_fail;
        }
    }
    state.eax = 0;
    return;

return_fail:
    state.eax = -1;
    return;
}

/** @brief The udriv_outb syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_outb_syscall(ureg_t state)
{
    struct {
        unsigned int port;
        unsigned char val;
    } args;

    tcb_t* tcb = get_tcb();
    ppd_t* ppd = tcb->process->directory;
    if (vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0) {
        goto return_fail;
    }
    if (check_port_permissions(tcb, args.port) < 0) {
        goto return_fail;
    }

    outb(args.port, args.val);

    state.eax = 0;
    return;

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
