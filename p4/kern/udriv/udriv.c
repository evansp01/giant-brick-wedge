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

// table of control entries for IDT entries
int_control_t interrupt_table[IDT_ENTS] = { { { 0 } } };

// global hashtable of all devices and servers
device_hash_t all_devices;

/** @brief Gets the device/server entry from global hashtable
 *  @param entry Driver ID to find
 *  @return Pointer to device entry in hashtable
 */
device_t *get_device(driv_id_t entry)
{
    return H_GET(&all_devices, entry, driver_id, global);
}

/** @brief Creates a device entry for a device/server
 *  @param id Device/server ID
 *  @return 0 on success, an integer less than 0 on failure
 */
device_t *create_device_entry(driv_id_t id)
{
    // create device entry
    device_t *device = (device_t*)smalloc(sizeof(device_t));
    if (device == NULL) {
        return NULL;
    }
    memset(device, 0, sizeof(device_t));
    device->driver_id = id;
    Q_INIT_ELEM(device, global);
    Q_INIT_ELEM(device, interrupts);
    Q_INIT_ELEM(device, tcb_link);
    return device;
}

void keyboard_interrupt();

void init_user_drivers()
{   
    // init global device/server hashtable
    if (H_INIT_TABLE(&all_devices) < 0) {
        panic("Cannot allocate global device hashtable");
    }

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
        device_t *device = create_device_entry(driv->id);
        if (device == NULL) {
            panic("Cannot malloc for device at %d", device_table[i].idt_slot);
        }
        
        // add to global hashtable of devices/servers
        assert(H_INSERT(&all_devices, device, driver_id, global) == NULL);
        
        // add device to list in interrupt table entry
        Q_INSERT_FRONT(&interrupt_table[driv->idt_slot].devices, device,
                       interrupts);
        interrupt_table[driv->idt_slot].num_devices++;
    }
}

void queue_interrupt(device_t* device)
{
    // TODO: queue the interrupt for the current device
}

void device_handler(ureg_t state)
{
    // we are using the ureg struct for device interrupts
    int idt_index = state.cause;

    if (idt_index == KEY_IDT_ENTRY) {
        keyboard_interrupt();
    }
    
    int_control_t* control = &interrupt_table[idt_index];
    device_t* device;
    // update all devices/servers registered to this IDT
    Q_FOREACH(device, &control->devices, interrupts)
    {
        // read from port if device requires it
        if (device->port != 0) {
            // TODO: need to read from port if there is one
        }
        
        // queue interrupt in the device/server buffer
        queue_interrupt(device);
    }
}
