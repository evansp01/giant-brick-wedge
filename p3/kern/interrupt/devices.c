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
#include <console.h>

#define TIMER_INTERRUPT_FREQUENCY 1000
#define KEYBOARD_BUFFER_SIZE 2048

/** @brief A circlular buffer for storing and reading keystrokes */
typedef struct {
    char buffer[KEYBOARD_BUFFER_SIZE];
    int producer; /* where the producer will next produce to */
    int consumer; /* where the consumer will next consume from */
    int num_chars;
    int num_newlines;
    int user_buf_len;
    tcb_t *readline_thread;
} keyboard_struct;

/** The circular buffer for the keyboard */
static keyboard_struct keyboard = {
    .producer = 0,
    .consumer = 0,
    .num_chars = 0,
    .num_newlines = 0,
    .user_buf_len = 0,
    .readline_thread = NULL
};

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

/** @brief The previous index in the circular keyboard buffer
 *
 *  @return The previous index
 *  */
static int prev_index(int index)
{
    if (index == 0) {
        return KEYBOARD_BUFFER_SIZE - 1;
    } else {
        return index - 1;
    }
}

/** @brief The next index in the circular keyboard buffer
 *
 *  @return The next index
 *  */
static int next_index(int index)
{
    return (index + 1) % KEYBOARD_BUFFER_SIZE;
}

static int is_readline()
{
    return (keyboard.user_buf_len != 0);
}

/** @brief Processes a single scancode into a character
 *  
 *  @param scancode Scancode to be processed
 *  @return processed character on success, -1 on failure
 *  */
static int readchar(uint8_t scancode)
{
    kh_type key = process_scancode(scancode);
    // not a real keypress
    if ((!KH_HASDATA(key))||(!KH_ISMAKE(key))) {
        return -1;
    }
    //it's a real character, return it!
    return KH_GETCHAR(key);
}

// TODO??: Should we move the timer interrupt to the scheduler and the
//         keyboard interrupt to readline? It seems like that is more where
//         they belong than in this file.

// TODO: '\r' character behavior

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
    char c;
    uint8_t scancode = inb(KEYBOARD_PORT);
    if ((c = readchar(scancode)) != -1) {
        // Backspace character
        if (c == '\b') {
            // If there are characters to delete
            if ((keyboard.num_chars != 0)&&
                (keyboard.buffer[prev_index(keyboard.producer)] != '\n')) {
                // Delete previous character from buffer
                keyboard.num_chars--;
                keyboard.producer = prev_index(keyboard.producer);
                // Echo deletion to console
                if (is_readline()){
                    putbyte(c);
                }
            }
        }
        // Standard character
        else {
            // If we aren't about to run into the consumer
            if (next_index(keyboard.producer) != keyboard.consumer) {
                // Add character to buffer
                keyboard.buffer[keyboard.producer] = c;
                keyboard.producer = next_index(keyboard.producer);
                keyboard.num_chars++;
                // Echo character to console
                if (is_readline()){
                    putbyte(c);
                }
            } else {
                //ignore the character, we don't have room
                //the program is more than KEYBOARD_BUFFER_SIZE characters
                //behind so it's probably okay.
            }
            if (c == '\n') {
                keyboard.num_newlines++;
            }
        }
        // Signal waiting readline thread if sufficient chars or newline
        if (is_readline()) {
            if ((keyboard.num_chars >= keyboard.user_buf_len)||
                (keyboard.num_newlines > 0)) {
                schedule(keyboard.readline_thread);
                keyboard.user_buf_len = 0;
                keyboard.readline_thread = NULL;
            }
        }
    }
    //ack interrupt
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

/** @brief Reads a line of up to the given length from the keyboard buffer
 *
 *  This function will block if there are less than 'len' characters in the
 *  buffer, and there are no newline characters. The keyboard interrupt handler
 *  will re-schedule the thread once either condition is met.
 *  
 *  @param len Length of user buffer
 *  @param buf User buffer to copy characters into
 *  @return number of characters copied into the user buffer
 *  */
int readline(int len, char *buf, tcb_t *tcb)
{   
    if ((keyboard.num_chars < len)&&(keyboard.num_newlines == 0)) {
        keyboard.user_buf_len = len;
        keyboard.readline_thread = tcb;
        // deschedule until a new line is available
        deschedule(tcb);
    }
    
    // Copy characters from keyboard buffer to user buffer
    int i;
    for (i = 0; i < len; i++) {
        buf[i] = keyboard.buffer[keyboard.consumer];
        keyboard.consumer = next_index(keyboard.consumer);
        keyboard.num_chars--;
        if (buf[i] == '\n') {
            keyboard.num_newlines--;
            return i + 1;
        }
    }
    return i;
}



/*
int readchar(void)
{
    //while we haven't run into the producer
    while (keyboard.consumer != keyboard.producer) {
        kh_type key = process_scancode(keyboard.buffer[keyboard.consumer]);
        keyboard.consumer = next_index(keyboard.consumer);
        keyboard.num_chars--;
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
*/
