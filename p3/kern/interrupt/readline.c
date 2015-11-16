/** @file readline.c
 *
 *  @brief Keyboard interrupt handler and code to handle readline requests
 *  
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (jonathao)
 *  @bug No known bugs
 **/
 
#include <stdlib.h>
#include <control.h>
#include <asm.h>
#include <keyhelp.h>
#include <console.h>
#include <scheduler.h>
#include <interrupt_defines.h>

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
        disable_interrupts();
        buf[i] = keyboard.buffer[keyboard.consumer];
        keyboard.consumer = next_index(keyboard.consumer);
        keyboard.num_chars--;
        if (buf[i] == '\n') {
            keyboard.num_newlines--;
            return i + 1;
        }
        enable_interrupts();
    }
    return i;
}