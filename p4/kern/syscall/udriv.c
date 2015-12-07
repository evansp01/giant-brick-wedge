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

/** @brief Check if the arguments for registering a hardware device are valid
 *  @param device Pointer to device entry
 *  @param in_port Port address if bytes are required
 *  @param in_bytes Number of bytes requested from port
 *  @return 0 on success, an integer less than 0 on failure
 */
int valid_hw_drv(devserv_t *device, unsigned int in_port, unsigned int in_bytes)
{   
    // device is already owned
    if (device->owner != NULL) {
        return -1;
    }
    // in_bytes must be 0 or 1 for a hardware device
    if (in_bytes > 1) {
        return -1;
    }
    // bytes are to be requested from port during interrupt
    if (in_bytes == 1) {
        const dev_spec_t* driv = device->device_table_entry;
        int i;
        for (i = 0; i < driv->port_regions_cnt; i++) {
            const udrv_region_t* port_region = &driv->port_regions[i];
            // in_port is valid for the device
            if (port_region->base == in_port) {
                return 0;
            }
        }
        // in_port is invalid for the given driver
        return -1;
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
void register_hw_drv(devserv_t *device, tcb_t* tcb, unsigned int in_port,
                     unsigned int in_bytes)
{
    // set request for bytes from port
    if (in_bytes == 1) {
        device->port = in_port;
        device->bytes = in_bytes;
    }
    // add device to current thread's tcb
    Q_INSERT_FRONT(&tcb->devserv, device, tcb_link);
    // set device owner
    device->owner = tcb;
}

/** @brief Check if the arguments for registering a server are valid
 *  @param driver_id ID of requested driver
 *  @param in_bytes Number of bytes requested
 *  @return 0 on success, an integer less than 0 on failure
 */
int valid_server(driv_id_t driver_id, unsigned int in_bytes)
{   
    // driver id must valid
    if ((driver_id <= UDR_MAX_HW_DEV)||(driver_id >= UDR_MIN_ASSIGNMENT)) {
        return -1;
    }
    // server must not already be owned
    if (driver_id != UDR_ASSIGN_REQUEST) {
        devserv_t *server = get_devserv(driver_id);
        if (server != NULL) {        
            if (server->owner != NULL) {
                return -1;
            }
        }
    }
    // in_bytes must be between 0 and sizeof(message_t)
    if (in_bytes > sizeof(message_t)) {
        return -1;
    }
    return 0;
}

/** @brief Register the server
 *  @param server Pointer to device entry
 *  @param tcb TCB of current thread
 *  @param in_bytes Number of bytes to read, if any
 *  @return void
 */
void register_server(devserv_t *server, tcb_t* tcb, unsigned int in_bytes)
{
    // set request for bytes
    if (in_bytes > 0) {
        server->bytes = in_bytes;
    }
    // add server to current thread's tcb
    Q_INSERT_FRONT(&tcb->devserv, server, tcb_link);
    // set server owner
    server->owner = tcb;
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
    ppd_t *ppd = tcb->process->directory;
    if(vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0){
        state.eax = -1;
        return;
    }
    
    // driver_id refers to a hardware device
    if (args.driver_id < UDR_MAX_HW_DEV) {
        devserv_t *device = get_devserv(args.driver_id);
        // device not in the device table
        if (device == NULL) {
            state.eax = -1;
            return;
        }
        if (valid_hw_drv(device, args.in_port, args.in_bytes) < 0) {
            state.eax = -1;
            return;
        } else {
            register_hw_drv(device, tcb, args.in_port, args.in_bytes);
            state.eax = device->driver_id;
            return;
        }
    }
    
    // driver_id refers to a user server
    // note: in_port is ignored for servers
    else {
        // check argument validity
        if (valid_server(args.driver_id, args.in_bytes) < 0) {
            state.eax = -1;
            return;
        } else {
            devserv_t *server;
            // user has requested to be assigned a server id
            if (args.driver_id == UDR_ASSIGN_REQUEST) {
                driv_id_t assigned_id = assign_driver_id();
                server = create_devserv_entry(assigned_id);
                add_devserv_global(server);
            }
            // driver_id is in list of defined servers
            else {
                server = get_devserv(args.driver_id);
                // server has not been created
                if (server == NULL) {
                    server = create_devserv_entry(args.driver_id);
                    add_devserv_global(server);
                }
            }
            register_server(server, tcb, args.in_bytes);
            state.eax = server->driver_id;
            return;
        }
    }
}

/** @brief The udriv_deregister syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_deregister_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_deregister. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}

/** @brief The udriv_send syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_send_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_send. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}

/** @brief The udriv_wait syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_wait_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_wait. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}

/** @brief The udriv_inb syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_inb_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_inb. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}

/** @brief The udriv_outb syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_outb_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_outb. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}

/** @brief The udriv_mmap syscall
 *  @param state The current state in user mode
 *  @return void
 */
void udriv_mmap_syscall(ureg_t state)
{
    KPRINTF("Thread %d called udriv_mmap. Not yet implemented.", get_tcb()->id);
    state.eax = -1;
}
