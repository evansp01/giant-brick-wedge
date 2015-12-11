#include "serial_console.h"

char newline[81];
const int newline_len = 81;
char backspace[] = { '\b', ' ', '\b' };
const int backspace_len = 3;

struct {
    int is_bs;
    const char* bs_loc;
    int bs_index;
    int bs_len;
} bs_buffer;

struct {
    int producer;
    int consumer;
    char buf[READLINE_MAX_LEN * 2];
} r_print = { .producer = 1, .consumer = 0 };

struct {
    mutex_t mutex;
    cond_t cvar;
    char buf[MAX_PRINT_LENGTH];
    int len;
    int index;
} printer;

void print_message(int len, int suggest_id)
{
    mutex_lock(&printer.mutex);
    printer.len = len;
    printer.index = 0;
    // we should suggest that the print driver prints
    udriv_send(suggest_id, 0, 0);
    cond_wait(&printer.cvar, &printer.mutex);
    mutex_unlock(&printer.mutex);
}

int get_print_char(char* c)
{
    int got = 0;
    mutex_lock(&printer.mutex);
    // if we can print something from the print buffer
    if (printer.index < printer.len) {
        *c = printer.buf[printer.index];
        printer.index++;
        if (printer.index == printer.len) {
            cond_signal(&printer.cvar);
        }
        got = 1;
        // otherwise, lets try the interrupt print buffer
    }
    mutex_unlock(&printer.mutex);
    return got;
}

void init_console()
{
    int i;
    newline[0] = '\n';
    for (i = 1; i < newline_len; i++) {
        newline[i] = '\b';
    }
    mutex_init(&printer.mutex);
    cond_init(&printer.cvar);
}

int get_bs_char(char* c)
{
    if (bs_buffer.bs_index != bs_buffer.bs_len) {
        *c = bs_buffer.bs_loc[bs_buffer.bs_index];
        bs_buffer.bs_index++;
        return 1;
    }
    return 0;
}

int r_index(int i)
{
    return (i + READLINE_MAX_LEN * 2) % (READLINE_MAX_LEN * 2);
}

int send_to_print(int len, char* buf)
{
    int i;
    for (i = 0; i < len; i++) {
        int next = r_index(r_print.producer + 1);
        if (next != r_print.consumer) {
            r_print.buf[r_print.producer] = buf[i];
            r_print.producer = next;
        }
    }
    return 0;
}

int poll_from_print(char* c)
{
    if (r_print.consumer != r_print.producer) {
        *c = r_print.buf[r_print.consumer];
        r_print.consumer = r_index(r_print.consumer + 1);
        return 1;
    }
    return 0;
}

int get_next_char(char* c)
{
    if (get_bs_char(c)) {
        return 1;
    }
    if (!poll_from_print(c)) {
        if(!get_print_char(c)){
            return 0;
        }
        //get print character
    }
    if (*c == '\n') {
        bs_buffer.bs_loc = (const char*)newline;
        bs_buffer.bs_len = newline_len;
        bs_buffer.bs_index = 0;
        get_bs_char(c);
        return 1;
    }
    if (*c == '\b') {
        bs_buffer.bs_loc = (const char*)backspace;
        bs_buffer.bs_len = backspace_len;
        bs_buffer.bs_index = 0;
        get_bs_char(c);
        return 1;
    }
    return 1;
}
