/** @file serial_server.c
 *
 *  @brief Serial server to handle user serial input
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
#include <ns16550.h>
#include <string.h>

/** @brief The size of the buffer that readline uses to store characters */
#define KEYBOARD_BUFFER_SIZE (READLINE_MAX_LEN * 2)
/** @brief The maximum number of characters a call to readline can take */
#define READLINE_MAX_LEN (80 * (24 - 1))

#define BUF_LEN 1024
#define COMMAND_CANCEL 1

typedef struct {
    driv_id_t device;
    int port;
} args_t;

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


#define BAUD_RATE 115200

#define LSB(val) GET_BYTE(val, 0)
#define MSB(val) GET_BYTE(val, 1)


void write_port(int port, reg_t reg, unsigned char value)
{
    udriv_outb(port + reg, value);
}

int read_port(int port, reg_t reg)
{
    unsigned char value;
    if (udriv_inb(port + reg, &value) < 0) {
        return -1;
    }
    return value;
}

void setup_serial_driver(int serial, int int_enable_flags)
{
    int rate = UART_CLOCK / BAUD_RATE;
    write_port(serial, REG_LINE_CNTL, LCR_DLAB);
    write_port(serial, REG_BAUD_LSB, LSB(rate));
    write_port(serial, REG_BAUD_MSB, MSB(rate));
    // setup the line control registry
    write_port(serial, REG_LINE_CNTL, CONF_8N1);
    // enable interrupts from the serial driver
    write_port(serial, REG_INT_EN, int_enable_flags);
}

void* interrupt_loop(void* args)
{
    char *arg = (char*)args;
    driv_id_t device;
    int port;
    driv_id_t driv_recv;
    message_t scancode;
    unsigned int size;

    if (strcmp(arg, "COM1") == 0) {
        device = UDR_DEV_COM1;
        port = COM1_IO_BASE;
    } else if (strcmp(arg, "COM2") == 0) {
        device = UDR_DEV_COM2;
        port = COM2_IO_BASE;
    } else if (strcmp(arg, "COM3") == 0) {
        device = UDR_DEV_COM3;
        port = COM3_IO_BASE;
    } else if (strcmp(arg, "COM4") == 0) {
        device = UDR_DEV_COM4;
        port = COM4_IO_BASE;
    } else {
        printf("unknown port %s", arg);
        return (void *)-1;
    }
    
    // register for serial driver
    if (udriv_register(device, port, 1) < 0) {
        printf("cannot register for serial driver");
        return (void *)-1;
    }
    // configure serial port
    setup_serial_driver(port, 0); // TODO: flags?
    
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
        lprintf("scancode: %d", (int)scancode);
        /*
        int c = readchar((uint8_t)scancode);
        if (c != -1) {
            add_readline_char((char)c);
        }
        */
    }
    return NULL;
}


int main(int argc, char **argv)
{
    char *port = argv[1];
    
    int pid;
    if ((pid = fork()) != 0) {
        if (pid < 0) {
            printf("serial server could not be started\n");
            return -1;
        } else {
            lprintf("started serial server with pid %d", pid);
            return 0;
        }
    }

    thr_init(4096);

    driv_id_t readline_server;
    //driv_id_t print_server;
    if (strcmp(port, "COM1") == 0) {
        readline_server = UDR_COM1_READLINE_SERVER;
        //print_server = UDR_COM1_PRINT_SERVER;
    } else if (strcmp(port, "COM2") == 0) {
        readline_server = UDR_COM2_READLINE_SERVER;
        //print_server = UDR_COM2_PRINT_SERVER;
    } else if (strcmp(port, "COM3") == 0) {
        readline_server = UDR_COM3_READLINE_SERVER;
        //print_server = UDR_COM3_PRINT_SERVER;
    } else if (strcmp(port, "COM4") == 0) {
        readline_server = UDR_COM4_READLINE_SERVER;
        //print_server = UDR_COM4_PRINT_SERVER;
    } else {
        printf("unknown port %s", port);
        return -1;
    }
    
    mutex_init(&keyboard.mutex);
    cond_init(&keyboard.cvar);

    // create readline user request thread
    thr_create(interrupt_loop, (void*)port);

    ipc_state_t* server_st;
    if (ipc_server_init(&server_st, readline_server) < 0) {
        printf("could not register for serial readline server, exiting...\n");
        return -1;
    }

    while (1) {
        /*
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
        */
    }
    // Should never get here
    return -1;
}