#include <ureg.h>
#include <keyhelp.h>
#include <simics.h>
#include <user_drivers.h>
#include <variable_queue.h>
#include <variable_htable.h>
#include <udriv_kern.h>
#include <stdlib.h>
#include <interrupt.h>
#include <string.h>
#include <idt.h>
#include <assert.h>
#include <asm.h>
#include <interrupt_defines.h>

// table of control entries for IDT entries
int_control_t interrupt_table[IDT_ENTS] = { { { 0 } } };

// global hashtable of all devices and servers
device_hash_t all_devserv;

// counter for kernel assigned driver ids
driv_id_t assigned_driver_id;

/** @brief Assigns a driver id
 *  @return Driver ID
 */
driv_id_t assign_driver_id()
{
    assigned_driver_id++;
    return assigned_driver_id;
}

/** @brief Gets the device/server entry from global hashtable
 *  @param entry Driver ID to find
 *  @return Pointer to device entry in hashtable
 */
devserv_t *get_devserv(driv_id_t entry)
{
    return H_GET(&all_devserv, entry, driver_id, global);
}

/** @brief Adds a device/server entry to the global hashtable
 *  @param device Pointer to device/server entry
 *  @return void
 */
void add_devserv_global(devserv_t *device)
{
    assert(H_INSERT(&all_devserv, device, driver_id, global) == NULL);
}

/** @brief Creates an entry for a device/server
 *  @param id Device/server ID
 *  @return 0 on success, an integer less than 0 on failure
 */
devserv_t *create_devserv_entry(driv_id_t id)
{
    // create device entry
    devserv_t *devserv = (devserv_t*)smalloc(sizeof(devserv_t));
    if (devserv == NULL) {
        return NULL;
    }
    memset(devserv, 0, sizeof(devserv_t));
    devserv->driver_id = id;
    Q_INIT_ELEM(devserv, global);
    Q_INIT_ELEM(devserv, interrupts);
    Q_INIT_ELEM(devserv, tcb_link);
    return devserv;
}

void keyboard_interrupt();

void init_user_drivers()
{   
    // init global device/server hashtable
    if (H_INIT_TABLE(&all_devserv) < 0) {
        panic("Cannot allocate global device hashtable");
    }
    
    assigned_driver_id = UDR_MIN_ASSIGNMENT;

    int i;
    for (i = 0; i < device_table_entries; i++) {
        const dev_spec_t* driv = &device_table[i];
        if (driv->idt_slot == UDR_NO_IDT) {
            continue;
        }
        // install the idt entry
        // note: multiple devices can share a single IDT entry
        if (install_user_device(driv->idt_slot) < 0) {
            // entry already exists in the IDT
            int_control_t* control = &interrupt_table[driv->idt_slot];
            if (control->num_devices == 0) {
                // we are attempting to overwrite a syscall handler in the IDT
                panic("Cannot install device at %d", device_table[i].idt_slot);
            }
        }
        
        // initialize the list of devices for table entry
        if (interrupt_table[driv->idt_slot].num_devices == 0) {
            Q_INIT_HEAD(&interrupt_table[driv->idt_slot].devices);
        }
        
        // create device entry
        devserv_t *device = create_devserv_entry(driv->id);
        if (device == NULL) {
            panic("Cannot malloc for device at %d", device_table[i].idt_slot);
        }
        
        // add to global hashtable of devices/servers
        add_devserv_global(device);
        
        // add device to list in interrupt table entry
        Q_INSERT_FRONT(&interrupt_table[driv->idt_slot].devices, device,
                       interrupts);
        interrupt_table[driv->idt_slot].num_devices++;
    }
}

/** @brief Gets the next index in the circular interrupt buffer
 *
 *  @param index The current index
 *  @return The next index
 **/
static int next_index(int index)
{
    return (index + 1) % INTERRUPT_BUFFER_SIZE;
}

/** @brief Queue the interrupt for the given device
 *  @param device The device to queue the interrupt for
 *  @param interrupt The interrupt to be queued
 *  @return void
 **/
void queue_interrupt(devserv_t* device, interrupt_t interrupt)
{
    // If we aren't about to run into the consumer
    if (next_index(device->producer) != device->consumer) {
        // Add character to buffer
        device->buffer[device->producer] = interrupt;
        device->producer = next_index(device->producer);
    } else {
        // ignore the interrupt, we don't have room
        // the program is more than INTERRUPT_BUFFER_SIZE interrupts
        // behind so it's probably okay
    }
}

void device_handler(ureg_t state)
{
    // we are using the ureg struct for device interrupts
    int idt_index = state.cause;

    if (idt_index == KEY_IDT_ENTRY) {
        keyboard_interrupt();
    }
    
    int_control_t* control = &interrupt_table[idt_index];
    devserv_t* device;
    // update all devices/servers registered to this IDT
    Q_FOREACH(device, &control->devices, interrupts)
    {
        // only forward interrupts for devices with registered drivers
        if (device->owner == NULL) {
            continue;
        }
        
        uint8_t read_byte;
        interrupt_t interrupt;
        // read from port if device requires it
        if (device->port != 0) {
            // devices can only read 1 byte from ports
            assert(device->bytes == 1);
            read_byte = inb(device->port);
            interrupt.msg = (message_t) read_byte;
            interrupt.size = device->bytes;
        }
        
        // queue interrupt in the device/server buffer
        queue_interrupt(device, interrupt);
    }
    // ack interrupt
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}
