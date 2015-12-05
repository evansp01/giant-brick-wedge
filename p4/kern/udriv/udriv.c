#include <ureg.h>
#include <keyhelp.h>
#include <simics.h>
#include <user_drivers.h>
#include <variable_queue.h>
#include <udriv_kern.h>
#include <stdlib.h>
#include <interrupt.h>

// TODO: 256 should be idt max or something
int_control_t interrupt_table[256] = { { 0 } };

// TODO: will we need a table like
// int device_owner[device_table_entries];
// which stores who owns each device so that we can ensure one person per
// device (though not one person per interrupt)

void keyboard_interrupt();

void init_user_drivers()
{
    int i;
    for (i = 0; i < device_table_entries; i++) {
        const dev_spec_t* driv = &device_table[i];
        if (driv->idt_slot == UDR_NO_IDT) {
            continue;
        }
        // install the idt entry
        if (install_user_device(driv->idt_slot) < 0) {
            panic("Unable to install device at %d", device_table[i].idt_slot);
        }
        // update the interrupt table
        Q_INIT_HEAD(&interrupt_table[driv->idt_slot].listeners);
        interrupt_table[driv->idt_slot].device_index = driv->id;
    }
}

void queue_interrupt(int_buf_t* buf)
{
    // queue the interrup
}

void device_handler(ureg_t state)
{
    // We are using the ureg struct for device interrupts
    int idt_index = state.cause;

    if (idt_index == KEY_IDT_ENTRY) {
        keyboard_interrupt();
    }
    int_control_t* control = &interrupt_table[idt_index];
    int_buf_t* buf;

    // TODO: need to read from port if there is one

    Q_FOREACH(buf, &control->listeners, link)
    {
        // TODO: should take more parameters, actually queue interrupt
        queue_interrupt(buf);
    }
}
