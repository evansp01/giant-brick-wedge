/** @file readline.c
 *
 *  @brief Keyboard interrupt handler and code to handle readline requests
 *
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (jonathao)
 *  @bug No known bugs
 **/

#include <stdlib.h>
#include <control_block.h>
#include <asm.h>
#include <keyhelp.h>
#include <console.h>
#include <scheduler.h>
#include <interrupt_defines.h>
#include <video_defines.h>

/** @brief The size of the buffer that readline uses to store characters */
#define KEYBOARD_BUFFER_SIZE (READLINE_MAX_LEN * 2)
/** @brief The maximum number of characters a call to readline can take */
#define READLINE_MAX_LEN (CONSOLE_WIDTH*(CONSOLE_HEIGHT-1))

/** @brief A temporary buffer used for copying characters to the user buffer */
static char temp[READLINE_MAX_LEN];
/** @brief The mutex which protects the readline system call */
static mutex_t read_mutex;

/** @brief A circlular buffer for storing and reading keystrokes */
struct {
    int producer; /* where the producer will next produce to */
    int consumer; /* where the consumer will next consume from */
    int num_chars;
    int num_newlines;
    int user_buf_len;
    int new_readline;
    tcb_t *readline_thread;
    char buffer[KEYBOARD_BUFFER_SIZE];
} keyboard = {0};

/** @brief The previous index in the circular keyboard buffer
 *
 *  @param index The index to get the previous index of
 *  @return The previous index
 *  */
static int prev_index(int index)
{
    return (index + KEYBOARD_BUFFER_SIZE - 1) % KEYBOARD_BUFFER_SIZE;
}

/** @brief The next index in the circular keyboard buffer
 *
 *  @param index The index to get the index after
 *  @return The next index
 *  */
static int next_index(int index)
{
    return (index + 1) % KEYBOARD_BUFFER_SIZE;
}

/** @brief Is there an outstanding call to readline
 *  @return A boolean integer
 **/
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

/** @brief Prints the current readline buffer
 *  @return void
 *  */
static void print_buffer()
{
    int current_index = keyboard.consumer;

    while (current_index != keyboard.producer) {
        putbyte(keyboard.buffer[current_index]);
        current_index = next_index(current_index);
    }
}

/** @brief Handle a ketboard interrupt, read the scancode and store in buffer
 *
 *  Reads the scancode from the keyboard and stores it in the keyboard buffer
 *  so it can later be read by readchar. This is the function which is called
 *  by the keyboard assembly wrapper
 *
 *  @param state The state of the process before the keyboard interrupt
 *
 *  @return void
 *  */
void keyboard_interrupt(ureg_t state)
{
    char c;
    uint8_t scancode = inb(KEYBOARD_PORT);
    if ((c = readchar(scancode)) == -1) {
        outb(INT_CTL_PORT, INT_ACK_CURRENT);
        return;
    }
    // Echo characters which were placed in buffer before readline call
    if (keyboard.new_readline) {
        print_buffer();
        keyboard.new_readline = 0;
    }
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
    // Standard character (ignore carriage return)
    else if (c != '\r') {
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
            keyboard.user_buf_len = 0;
            schedule(keyboard.readline_thread, T_KERN_SUSPENDED);
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
 *  @param tcb The tcb of the thread calling readline
 *  @return number of characters copied into the user buffer
 *  */
int readline(int len, char *buf, tcb_t *tcb)
{
    int echo = 0;
    if ((keyboard.num_chars < len)&&(keyboard.num_newlines == 0)) {
        disable_interrupts();
        keyboard.user_buf_len = len;
        keyboard.readline_thread = tcb;
        keyboard.new_readline = 1;
        // deschedule until a new line is available
        deschedule(tcb, T_KERN_SUSPENDED);
    } else {
        echo = 1;
    }

    // Copy characters from keyboard buffer to user buffer
    int i;
    for (i = 0; i < len; i++) {

        // Get the character from the keyboard buffer
        disable_interrupts();
        temp[i] = keyboard.buffer[keyboard.consumer];
        keyboard.consumer = next_index(keyboard.consumer);
        keyboard.num_chars--;
        if (temp[i] == '\n') {
            keyboard.num_newlines--;
        }
        enable_interrupts();

        // Done if we encounter a newline
        if (temp[i] == '\n') {
            i++;
            break;
        }
    }

    // Copy to the user buffer
    ppd_t *ppd = tcb->process->directory;
    if (vm_write_locked(ppd, temp, (uint32_t)buf, i*sizeof(char)) < 0) {
        return -1;
    }
    // Echo bytes if the keyboard handler hasn't already done so
    if (echo) {
        putbytes(temp, i);
    }
    return i;
}

/** @brief Initializes the mutexes used in the console syscalls
 *  @return void
 */
void init_readline()
{
    mutex_init(&read_mutex);
}

/** @brief The readline syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readline_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;
    struct {
        int len;
        char *buf;
    } args;

    if(vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0){
        state.eax = -1;
        return;
    }

    if(args.len == 0){
        state.eax = 0;
        return;
    }

    // Error: len is unreasonable
    if ((args.len > READLINE_MAX_LEN)||(args.len < 0)) {
        state.eax = -1;
        return;
    }
    // Error: buf is not a valid memory address
    mutex_lock(&ppd->lock);
    if (!vm_user_can_write(ppd, (void *)args.buf, args.len)) {
        mutex_unlock(&ppd->lock);
        state.eax = -1;
        return;
    }
    mutex_unlock(&ppd->lock);

    mutex_lock(&read_mutex);
    int num_bytes = readline(args.len, args.buf, tcb);
    mutex_unlock(&read_mutex);

    state.eax = num_bytes;
}
