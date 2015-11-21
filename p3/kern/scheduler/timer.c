/** @file timer.c
 *
 *  @brief Handler for the timer interrupts.
 *
 *  The timer interrupt handler simply increments a count of timer ticks.
 *
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 *  */

#include <asm.h>
#include <scheduler.h>
#include <control_block.h>
#include <interrupt_defines.h>
#include <timer_defines.h>
#include <simics.h>

/** @brief The frequency with which timer interrupts should occur */
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
void init_timer()
{
    uint16_t frequency = 1 + (TIMER_RATE / TIMER_INTERRUPT_FREQUENCY);
    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    outb(TIMER_PERIOD_IO_PORT, (uint8_t)frequency);
    outb(TIMER_PERIOD_IO_PORT, (uint8_t)(frequency >> 8));
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
