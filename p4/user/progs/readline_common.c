/** @file readline_common.c
 *
 *  @brief Common readline functions used in serial and readline drivers
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
#include "readline_common.h"

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
static int is_readline(keyboard_t* keyboard)
{
    return (keyboard->user_buf_len != 0);
}

/** @brief Has readline produced a new line for the server to consume
 *
 *  @param keyboard The readline keyboard
 *  @return a boolean integer
 **/
static int readline_ready(keyboard_t* keyboard)
{
    if (!is_readline(keyboard))
        return 0;
    if (keyboard->num_newlines > 0)
        return 1;
    if (keyboard->num_chars >= keyboard->user_buf_len)
        return 1;
    return 0;
}

/** @brief Prints the current readline buffer
 *  @return void
 *  */
static void print_buffer(keyboard_t* keyboard, print_func_t pf)
{
    int current_index = keyboard->consumer;
    while (current_index != keyboard->producer) {
        pf(1, &keyboard->buffer[current_index]);
        current_index = next_index(current_index);
    }
}

/** @brief Handle a backspace character for readline 
 *
 *  @param keyboard The keyboard structure used to fufill this request
 *  @param c The backspace character
 *  @param pf The function used to echo characters
 *  @return void
 **/
void backspace_char(keyboard_t* keyboard, char c, print_func_t pf)
{
    // If there are characters to delete
    if (keyboard->num_chars == 0) {
        return;
    }
    if (keyboard->buffer[prev_index(keyboard->producer)] == '\n') {
        return;
    }
    atomic_dec(&keyboard->num_chars);
    keyboard->producer = prev_index(keyboard->producer);
    // Echo deletion to console
    if (is_readline(keyboard)) {
        pf(1, &c);
    }
}

/** @brief Handle a normal character for readline 
 *
 *  @param keyboard The keyboard structure used to fufill this request
 *  @param c The character (anything but a backspace)
 *  @param pf The function used to echo characters
 *  @return void
 **/
void regular_char(keyboard_t* keyboard, char c, print_func_t pf)
{
    // ignore carriage return characters since they are hard to deal with
    if (c == '\r') {
        return;
    }
    // If we aren't about to run into the consumer
    if (next_index(keyboard->producer) != keyboard->consumer) {
        // Add character to buffer
        keyboard->buffer[keyboard->producer] = c;
        keyboard->producer = next_index(keyboard->producer);
        atomic_inc(&keyboard->num_chars);
        // Echo character to console
        if (is_readline(keyboard)) {
            pf(1, &c);
        }
    } else {
        //ignore the character, we don't have room
        //the program is more than KEYBOARD_BUFFER_SIZE characters
        //behind so it's probably okay.
    }
    if (c == '\n') {
        atomic_inc(&keyboard->num_newlines);
    }
}

/** @brief Add a character to the current readline request
 *
 *  @param keyboard The keyboard structure used to fufill this request
 *  @param c The character
 *  @param pf The function used to echo characters
 *  @return void
 **/
void handle_char(keyboard_t* keyboard, char c, print_func_t pf)
{
    // Echo characters which were placed in buffer before readline call
    if (keyboard->new_readline) {
        print_buffer(keyboard, pf);
        keyboard->new_readline = 0;
    }
    // Backspace character
    if (c == '\b') {
        backspace_char(keyboard, c, pf);
    } else {
        regular_char(keyboard, c, pf);
    }
    // Signal waiting readline thread if sufficient chars or newline
    mutex_lock(&keyboard->mutex);
    if (readline_ready(keyboard)) {
        keyboard->user_buf_len = 0;
        cond_signal(&keyboard->cvar);
    }
    mutex_unlock(&keyboard->mutex);
}

/** @brief Handles a readline request by filling the user buffer with a line
 *
 *  @param keyboard The keyboard structure used to fufill this request
 *  @param buf The buffer to store the line in
 *  @param len The length of the line requested
 *  @param pf The function used to echo characters
 *  @return -1 on failure, the line length on success
 **/
int handle_request(keyboard_t* keyboard, char* buf, int len, print_func_t pf)
{
    if (len > READLINE_MAX_LEN) {
        return -1;
    }
    int echo = 0;
    mutex_lock(&keyboard->mutex);
    if ((keyboard->num_chars < len) && (keyboard->num_newlines == 0)) {
        keyboard->user_buf_len = len;
        keyboard->new_readline = 1;
        // deschedule until a new line is available
        cond_wait(&keyboard->cvar, &keyboard->mutex);
    } else {
        echo = 1;
    }
    mutex_unlock(&keyboard->mutex);

    // Copy characters from keyboard buffer to user buffer
    int i;
    for (i = 0; i < len; i++) {

        // Get the character from the keyboard buffer
        buf[i] = keyboard->buffer[keyboard->consumer];
        keyboard->consumer = next_index(keyboard->consumer);
        atomic_dec(&keyboard->num_chars);
        if (buf[i] == '\n') {
            atomic_dec(&keyboard->num_newlines);
        }

        // Done if we encounter a newline
        if (buf[i] == '\n') {
            i++;
            break;
        }
    }
    // Echo bytes if the keyboard handler hasn't already done so
    if (echo) {
        pf(i, buf);
    }
    return i;
}

/** @brief Initializes a keyboard structure
 *  @param keyboard The keyboard structure to initialize
 *  @return void
 **/
void init_keyboard(keyboard_t* keyboard)
{
    mutex_init(&keyboard->mutex);
    cond_init(&keyboard->cvar);
}
