/** @file readline_common.h
 *
 *  @brief Interface for common functions for readline servers
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef H_READLINE_COMMON
#define H_READLINE_COMMON

#include <keyhelp.h>

/** @brief The size of the buffer that readline uses to store characters */
#define KEYBOARD_BUFFER_SIZE (READLINE_MAX_LEN * 2)
/** @brief The maximum number of characters a call to readline can take */
#define READLINE_MAX_LEN (80 * (24 - 1))

/** @brief A circlular buffer for storing and reading keystrokes */
typedef struct keyboard {
    int producer; /* where the producer will next produce to */
    int consumer; /* where the consumer will next consume from */
    volatile int num_chars;
    int num_newlines;
    int user_buf_len;
    int new_readline;
    char buffer[KEYBOARD_BUFFER_SIZE];
    mutex_t mutex;
    cond_t cvar;
} keyboard_t;

typedef int (*print_func_t)(int len, char* buf);

void init_keyboard(keyboard_t *keyboard);
void handle_char(keyboard_t* keyboard, char c, print_func_t pf);
int handle_request(keyboard_t* keyboard, char* buf, int len, print_func_t pf);

/* This is terrible style, but the build system leaves us will few choices
 * other than re-implementing the same functions in serial server and
 * readline server. That option is worse.
 **/
#include "readline_common.c"

#endif //H_READLINE_COMMON
