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

#include <asm.h>
#include <scheduler.h>
#include <control.h>
#include <interrupt_defines.h>
#include <timer_defines.h> 

#define TIMER_INTERRUPT_FREQUENCY 1000

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
int initialize_devices()
{
    uint16_t frequency = 1 + (TIMER_RATE / TIMER_INTERRUPT_FREQUENCY);
    install_devices();
    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    outb(TIMER_PERIOD_IO_PORT, (uint8_t)frequency);
    outb(TIMER_PERIOD_IO_PORT, (uint8_t)(frequency >> 8));
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
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    run_scheduler(ticks_so_far);
}
