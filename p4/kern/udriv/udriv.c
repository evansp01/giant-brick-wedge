#include <ureg.h>
#include <timer_defines.h>
#include <keyhelp.h>
#include <simics.h>

void timer_interrupt();
void keyboard_interrupt();

void device_handler(ureg_t state) {
    // We are using the ureg struct for device interrupts
    int idt_index = state.cause;

    if(idt_index == TIMER_IDT_ENTRY) {
        timer_interrupt();
    }
    if(idt_index == KEY_IDT_ENTRY){
        keyboard_interrupt();

    }

}
