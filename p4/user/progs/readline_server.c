/** @file readline_server.c
 *
 *  @brief Readline server to handle user readline requests
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <stdint.h>
#include <keyhelp.h>
#include <atomic.h>
#include <mutex.h>
#include <cond.h>
#include <thread.h>
#include <thrgrp.h>
#include <stdlib.h>
#include <ipc_server.h>
#include <stdio.h>
#include <simics.h>

/** @brief The size of the buffer that readline uses to store characters */
#define KEYBOARD_BUFFER_SIZE (READLINE_MAX_LEN * 2)
/** @brief The maximum number of characters a call to readline can take */
#define READLINE_MAX_LEN (80 * (24 - 1))

#define BUF_LEN 1024
#define COMMAND_CANCEL 1

typedef union {
    message_t raw;
    struct {
        unsigned int sender;
        unsigned short len;
        unsigned char cmd;
        unsigned char byte;
    };
} request_msg_t;

/** @brief A circlular buffer for storing and reading keystrokes */
struct {
    int producer; /* where the producer will next produce to */
    int consumer; /* where the consumer will next consume from */
    volatile int num_chars;
    int num_newlines;
    int user_buf_len;
    int new_readline;
    char buffer[KEYBOARD_BUFFER_SIZE];
    mutex_t mutex;
    cond_t cvar;
} keyboard = { 0 };

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

static int readline_ready()
{
    if (!is_readline())
        return 0;
    if (keyboard.num_newlines > 0)
        return 1;
    if (keyboard.num_chars >= keyboard.user_buf_len)
        return 1;
    return 0;
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
    if ((!KH_HASDATA(key)) || (!KH_ISMAKE(key))) {
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
        print(1, &keyboard.buffer[current_index]);
        current_index = next_index(current_index);
    }
}

void backspace_char(char c)
{
    // If there are characters to delete
    if (keyboard.num_chars == 0) {
        return;
    }
    if (keyboard.buffer[prev_index(keyboard.producer)] == '\n') {
        return;
    }
    atomic_dec(&keyboard.num_chars);
    keyboard.producer = prev_index(keyboard.producer);
    // Echo deletion to console
    if (is_readline()) {
        print(1, &c);
    }
}

void regular_char(char c)
{
    // ignore carriage return characters since they are hard to deal with
    if (c == '\r') {
        return;
    }
    // If we aren't about to run into the consumer
    if (next_index(keyboard.producer) != keyboard.consumer) {
        // Add character to buffer
        keyboard.buffer[keyboard.producer] = c;
        keyboard.producer = next_index(keyboard.producer);
        atomic_inc(&keyboard.num_chars);
        // Echo character to console
        if (is_readline()) {
            print(1, &c);
        }
    } else {
        //ignore the character, we don't have room
        //the program is more than KEYBOARD_BUFFER_SIZE characters
        //behind so it's probably okay.
    }
    if (c == '\n') {
        atomic_inc(&keyboard.num_newlines);
    }
}

void add_readline_char(char c)
{
    // Echo characters which were placed in buffer before readline call
    if (keyboard.new_readline) {
        print_buffer();
        keyboard.new_readline = 0;
    }
    // Backspace character
    if (c == '\b') {
        backspace_char(c);
    } else {
        regular_char(c);
    }
   // Signal waiting readline thread if sufficient chars or newline
    mutex_lock(&keyboard.mutex);
    if (readline_ready()) {
        keyboard.user_buf_len = 0;
        cond_signal(&keyboard.cvar);
    }
    mutex_unlock(&keyboard.mutex);
}

void* interrupt_loop(void* arg)
{
    driv_id_t driv_recv;
    message_t scancode;
    unsigned int size;

    // register for keyboard driver
    if (udriv_register(UDR_KEYBOARD, KEYBOARD_PORT, 1) < 0) {
        printf("cannot register for keyboard driver");
        return (void *)-1;
    }

    while (true) {
        // get scancode
        if (udriv_wait(&driv_recv, &scancode, &size) < 0) {
            printf("user keyboard interrupt handler failed to get scancode");
            return (void *)-1;
        }
        if (driv_recv != UDR_KEYBOARD) {
            printf("received interrupt from unexpected source");
            return (void *)-1;
        }
        int c = readchar((uint8_t)scancode);
        if (c != -1) {
            add_readline_char((char)c);
        }
    }
    return NULL;
}

int handle_user_request(char* buf, int len)
{
    int echo = 0;
    mutex_lock(&keyboard.mutex);
    if ((keyboard.num_chars < len) && (keyboard.num_newlines == 0)) {
        keyboard.user_buf_len = len;
        keyboard.new_readline = 1;
        // deschedule until a new line is available
        cond_wait(&keyboard.cvar, &keyboard.mutex);
    } else {
        echo = 1;
    }
    mutex_unlock(&keyboard.mutex);

    // Copy characters from keyboard buffer to user buffer
    int i;
    for (i = 0; i < len; i++) {

        // Get the character from the keyboard buffer
        buf[i] = keyboard.buffer[keyboard.consumer];
        keyboard.consumer = next_index(keyboard.consumer);
        atomic_dec(&keyboard.num_chars);
        if (buf[i] == '\n') {
            atomic_dec(&keyboard.num_newlines);
        }

        // Done if we encounter a newline
        if (buf[i] == '\n') {
            i++;
            break;
        }
    }
    // Echo bytes if the keyboard handler hasn't already done so
    if (echo) {
        print(i, buf);
    }
    return i;
}

int main()
{

    int pid;
    if ((pid = fork()) != 0) {
        if (pid < 0) {
            printf("readline server could not be started\n");
            return -1;
        } else {
            return 0;
        }
    }

    thr_init(4096);

    mutex_init(&keyboard.mutex);
    cond_init(&keyboard.cvar);

    // create readline user request thread
    thr_create(interrupt_loop, NULL);

    ipc_state_t* server_st;
    if (ipc_server_init(&server_st, UDR_READLINE_SERVER) < 0) {
        printf("could not register for readline server, exiting...\n");
        return -1;
    }

    while (1) {
        // receive a readline request
        driv_id_t sender;
        int len;
        if (ipc_server_recv(server_st, &sender, &len, sizeof(int), true) < 0) {
            printf("could not receive request, exiting...\n");
            ipc_server_cancel(server_st);
            return -1;
        }
        // request is too large
        if (len > READLINE_MAX_LEN) {
            request_msg_t req;
            req.cmd = COMMAND_CANCEL;
            udriv_send(sender, req.raw, sizeof(request_msg_t));
            continue;
        }
        // process request
        char buf[READLINE_MAX_LEN];
        int msg_len = handle_user_request((char*)&buf, len);
        ipc_server_send_msg(server_st, sender, &buf, msg_len);
    }
    // Should never get here
    return -1;
}
