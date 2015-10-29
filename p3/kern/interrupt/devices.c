/** @file handlers.c
 *
 *  @brief Handlers for the timer and keyboard interrupts.
 *
 *  The timer interrupt handler simply increments a count of timer ticks.
 *
 *  The keyboard interrupt handler pushes the keyboard scancodes into a
 *  circular buffer where they can later be read by readchar(). This allows
 *  the implmentation of reachar to read characters without disabling
 *  interrupts since it can just see if the consumer index into the buffer
 *  points to something which has been written by the producer. The producer
 *  will never try to access memory the consumer could be reading from.
 *
 *  Also included is code for installing the interrupt handlers. This confugres
 *  the timer to tick at a certain rate, and packs the idt entries
 *
 *  Limitations:
 *      If the user enters more than KEYBOARD_BUFFER_SIZE characters before
 *      the program calls readchar, characters will be lost. This seems fine
 *      since it should take a long time for the user to enter many characters
 *      and if the program simply isn't interested, perhaps it's better not to
 *      buffer an absurd number of characters. After all, what can you do with
 *      hour old keyboard input
 *
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 *  */

#include <stdio.h>
#include <asm.h>
#include <interrupt_defines.h>
#include <timer_defines.h>
#include <keyhelp.h>
#include <ctype.h>
#include <setup_idt.h>
#include <simics.h>

#include <loader.h>
#include <stdlib.h>
#include <ureg.h>
#include <control.h>
#include <switch.h>
#include <scheduler.h>

#define TIMER_INTERRUPT_FREQUENCY 1000
#define KEYBOARD_BUFFER_SIZE 2048

/** @brief A circlular buffer for storing and reading keystrokes */
typedef struct {
    uint8_t buffer[KEYBOARD_BUFFER_SIZE];
    int producer; /* where the producer will next produce to */
    int consumer; /* where the consumer will next consume from */
} keyboard_struct;

/** The circular buffer for the keyboard */
static keyboard_struct keyboard = {
    .producer = 0,
    .consumer = 0
};

/** @brief The current tick callback as set in handler_install */
static void (*tick_handler)(unsigned int);

/** @brief The number of ticks which have occured so far */
static unsigned int ticks_so_far;

/** @brief Setup the timer interrupt handler
 *
 *  Instruct the timer to fire at TIMER_INTERRUPT_FREQUENCY, and then pack
 *  the timer interrupt idt entry. Note that the offset used is for the
 *  assembly wrapper
 *
 *  @return void
 *  */
int initialize_devices(void (*tickback)(unsigned int))
{
    tick_handler = tickback;
    uint16_t frequency = 1 + (TIMER_RATE / TIMER_INTERRUPT_FREQUENCY);
    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    outb(TIMER_PERIOD_IO_PORT, (uint8_t)frequency);
    outb(TIMER_PERIOD_IO_PORT, (uint8_t)(frequency >> 8));
    install_devices();
    return 0;
}

/** @brief Handler a timer interrupt, accumulate ticks
 *
 *  This is the function which is called by the timer assembly wrapper
 *
 *  @return void
 *  */
void timer_interrupt()
{
    ticks_so_far++;
    tick_handler(ticks_so_far);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    
    // let timer trigger the scheduler
    run_next();
}

/** @brief The next index in the circular keyboard buffer
 *
 *  @return The next index
 *  */
static int next_index(int index)
{
    return (index + 1) % KEYBOARD_BUFFER_SIZE;
}

// TODO: Remove once testing is complete
int keyboard_count = 0;

/** @brief Handle a ketboard interrupt, read the scancode and store in buffer
 *
 *  Reads the scancode from the keyboard and stores it in the keyboard buffer
 *  so it can later be read by readchar. This is the function which is called
 *  by the keyboard assembly wrapper
 *
 *  @return void
 *  */
void keyboard_interrupt(ureg_t state)
{
    // Temporarily use keyboard interrupts to trigger context switchers
    inb(KEYBOARD_PORT);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    
    if (keyboard_count % 2) {
        yield(-1);
    }
    keyboard_count++;
    
    
    /*
    //if we aren't about to run into the consumer
    if (next_index(keyboard.producer) != keyboard.consumer) {
        keyboard.buffer[keyboard.producer] = inb(KEYBOARD_PORT);
        keyboard.producer = next_index(keyboard.producer);
    } else {
        //ignore the keypress, we don't have room.
        //the program is more than KEYBOARD_BUFFER_SIZE characters behind
        //so it's probably okay.
    }
    
    //ack interrupt
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    */
}

int readchar(void)
{
    //while we haven't run into the producer
    while (keyboard.consumer != keyboard.producer) {
        kh_type key = process_scancode(keyboard.buffer[keyboard.consumer]);
        keyboard.consumer = next_index(keyboard.consumer);
        //If this doesn't have data, keep looking
        if (!KH_HASDATA(key)) {
            continue;
        }
        //if this isn't a keypress, keep looking
        if (!KH_ISMAKE(key)) {
            continue;
        }
        //it's a real character, return it!
        return KH_GETCHAR(key);
    }
    return -1;
}
