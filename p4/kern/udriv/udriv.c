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
#include <control_block_struct.h>
#include <scheduler.h>
#include <atomic.h>

// table of control entries for IDT entries
int_control_t interrupt_table[IDT_ENTS] = { { { 0 } } };

/** @brief Struct for global device/server hashtable and lock */
typedef struct g_devserv {
    mutex_t mutex;
    device_hash_t all_devserv;
} g_devserv_t;

// global hashtable and lock
g_devserv_t all_ds;

// counter for kernel assigned driver ids
int assigned_driver_id;

/** @brief Assigns a driver id
 *  @return Driver ID
 */
int assign_driver_id()
{
    // Atomically get current value and increment
    return atomic_xadd(&assigned_driver_id, 1);
}

/** @brief Gets the device/server entry from global hashtable
 *  @param entry Driver ID to find
 *  @return Pointer to device entry in hashtable
 */
devserv_t *get_devserv(driv_id_t entry)
{
    mutex_lock(&all_ds.mutex);
    devserv_t *devserv = H_GET(&all_ds.all_devserv, entry, driver_id, global);
    mutex_unlock(&all_ds.mutex);
    return devserv;
}

/** @brief Adds a device/server entry to the global hashtable
 *  @param entry Pointer to device/server entry
 *  @return void
 */
void add_devserv(devserv_t *entry)
{
    mutex_lock(&all_ds.mutex);
    assert(H_INSERT(&all_ds.all_devserv, entry, driver_id, global) == NULL);
    mutex_unlock(&all_ds.mutex);
}

/** @brief Adds a device/server entry to the global hashtable if currently null
 *  @param entry Pointer to device/server entry
 *  @return 0 if entry was added, an integer less than 0 if conflict occurred
 */
int check_add_devserv(devserv_t *entry)
{
    mutex_lock(&all_ds.mutex);
    if (H_GET(&all_ds.all_devserv, entry->driver_id, driver_id, global)==NULL) {
        assert(H_INSERT(&all_ds.all_devserv, entry, driver_id, global)==NULL);
        mutex_unlock(&all_ds.mutex);
        return 0;
    } else {
        mutex_unlock(&all_ds.mutex);
        return -1;
    }
}

/** @brief Removes a device/server entry from the global hashtable
 *  @param entry Pointer to device/server entry
 *  @return void
 */
void remove_devserv(devserv_t *entry)
{
    mutex_lock(&all_ds.mutex);
    assert(H_REMOVE(&all_ds.all_devserv, entry->driver_id, driver_id, global)
           != NULL);
    mutex_unlock(&all_ds.mutex);
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
    mutex_init(&devserv->mutex);
    return devserv;
}

/** @brief Free an entry for a device/server
 *  @param entry Device/server entry to free
 *  @return void
 */
void free_devserv_entry(devserv_t *entry)
{
    mutex_destroy(&entry->mutex);
    sfree(entry, sizeof(devserv_t));
}

void keyboard_interrupt();

void init_user_drivers()
{   
    // init global device/server hashtable
    mutex_init(&all_ds.mutex);
    if (H_INIT_TABLE(&all_ds.all_devserv) < 0) {
        panic("Cannot allocate global device hashtable");
    }
    
    assigned_driver_id = UDR_MIN_ASSIGNMENT + 1;

    int i;
    for (i = 0; i < device_table_entries; i++) {
        const dev_spec_t* driv = &device_table[i];
        // create device entry
        devserv_t *device = create_devserv_entry(driv->id);
        if (device == NULL) {
            panic("Cannot malloc for device at %d", device_table[i].idt_slot);
        }
        device->device_table_entry = driv;
        // add to interrupt table if device has interrupts
        if (driv->idt_slot != UDR_NO_IDT) {
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
            // add device to list in interrupt table entry
            Q_INSERT_FRONT(&interrupt_table[driv->idt_slot].devices, device,
                           interrupts);
            interrupt_table[driv->idt_slot].num_devices++;
        }
        // add to global hashtable of devices/servers
        add_devserv(device);
    }
}

/** @brief Gets the next index in the circular interrupt buffer
 *
 *  @param index The current index
 *  @return The next index
 **/
int next_index_int(int index)
{
    return (index + 1) % INTERRUPT_BUFFER_SIZE;
}

/** @brief Queue the interrupt for the given device
 *  @param tcb The thread which will receive the interrupt
 *  @param interrupt The interrupt to be queued
 *  @return void
 **/
void queue_interrupt(tcb_t *tcb, interrupt_t interrupt)
{
    disable_interrupts();
    // if we aren't about to run into the consumer
    if (next_index_int(tcb->producer) != tcb->consumer) {
        // add character to buffer
        tcb->buffer[tcb->producer] = interrupt;
        tcb->producer = next_index_int(tcb->producer);
    } else {
        // ignore the interrupt, we don't have room
        // the program is more than INTERRUPT_BUFFER_SIZE interrupts
        // behind so it's probably okay
    }
    // signal the thread if it was waiting on an interrupt
    if (tcb->waiting == 1) {
        tcb->waiting = 0;
        schedule(tcb, T_KERN_SUSPENDED);
    }
    enable_interrupts();
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
        interrupt.driver_id = device->driver_id;
        // read from port if device requires it
        if (device->port != 0) {
            // devices can only read 1 byte from ports
            assert(device->bytes == 1);
            read_byte = inb(device->port);
            interrupt.msg = (message_t) read_byte;
            interrupt.size = device->bytes;
        } else {
            interrupt.msg = 0;
            interrupt.size = 0;
        }
        
        // queue interrupt in the device/server buffer
        queue_interrupt(device->owner, interrupt);
    }
    // ack interrupt
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}
