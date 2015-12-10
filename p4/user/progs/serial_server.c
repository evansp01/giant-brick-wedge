/** @file readline_server.c
 *
 *  @brief Readline server to handle user readline requests
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <stdint.h>
#include <thread.h>
#include <thrgrp.h>
#include <stdlib.h>
#include <string.h>
#include <ipc_server.h>
#include <syscall.h>
#include <ns16550.h>
#include <stdio.h>
#include <simics.h>
#include "readline_common.h"

#define BUF_LEN 1024
#define COMMAND_CANCEL 1
#define MOD_CNTL_MASTER_INT 8
#define INTERRUPTS \
    (IER_RX_FULL_INT_EN | IER_TX_EMPTY_INT_EN | IER_RECV_LINE_STAT_INT_EN)

typedef union {
    message_t raw;
    struct {
        unsigned int sender;
        unsigned short len;
        unsigned char cmd;
        unsigned char byte;
    };
} request_msg_t;

struct {
    driv_id_t driver_id;
    driv_id_t keyboard_id;
    ipc_state_t* state;
    port_t com_port;
    keyboard_t keyboard;
} serial_driver;

#define BAUD_RATE 115200

#define LSB(val) (val & 0xFF)
#define MSB(val) ((val >> 8) & 0xFF)

void write_port(port_t port, reg_t reg, int value)
{
    if (udriv_outb(port + reg, value) < 0) {
        lprintf("ut oh");
    }
}

int read_port(port_t port, reg_t reg)
{
    unsigned char val;
    if (udriv_inb(port + reg, &val) < 0) {
        lprintf("oh noes");
    }
    return val;
}

char readchar(int scan) {
    // in case it starts acting up again
    //if(scan == '&')
    //    return '\b';
    //if(scan == '+')
    //    return '\n';
    return scan;
}

char temp_buffer[READLINE_MAX_LEN + 1];
int send_to_print(int len, char* buf)
{
    memcpy(temp_buffer, buf, len);
    temp_buffer[len] = '\0';
    lprintf("SERIAL %s", temp_buffer);
    return 0;
}

void* interrupt_loop(void* arg)
{
    driv_id_t driv_recv;
    message_t scancode;
    unsigned int size;

    // register for keyboard driver
    lprintf("lets register for %d", serial_driver.keyboard_id);
    if (udriv_register(serial_driver.keyboard_id,
                       serial_driver.com_port + REG_LINE_CNTL, 1) < 0) {
        printf("cannot register for com driver");
        return (void*)-1;
    }

    int rate = UART_CLOCK / BAUD_RATE;
    write_port(serial_driver.com_port, REG_LINE_CNTL, LCR_DLAB);
    write_port(serial_driver.com_port, REG_BAUD_LSB, LSB(rate));
    write_port(serial_driver.com_port, REG_BAUD_MSB, MSB(rate));
    // setup the line control registry
    write_port(serial_driver.com_port, REG_LINE_CNTL, CONF_8N1);
    // enable interrupts from the serial driver
    write_port(serial_driver.com_port, REG_INT_EN, INTERRUPTS);
    write_port(serial_driver.com_port, REG_MOD_CNTL, MOD_CNTL_MASTER_INT);

    while (1) {
        // get scancode
        if (udriv_wait(&driv_recv, &scancode, &size) < 0) {
            printf("user keyboard interrupt handler failed to get scancode");
            return (void*)-1;
        }
        if (driv_recv != serial_driver.keyboard_id) {
            printf("received interrupt from unexpected source");
            return (void*)-1;
        }
        int cause = read_port(serial_driver.com_port, REG_INT_ID);
        if(cause & IIR_INT_TYPE_TX){
            // we should print
        }
        if(cause & IIR_INT_TYPE_RX) {
            char c = readchar(read_port(serial_driver.com_port, REG_DATA));
            lprintf("scan %c", c);
            handle_char(&serial_driver.keyboard, c, send_to_print);
        }

    }
    return NULL;
}

int setup_serial_driver(char* com)
{
    if (strcmp(com, "COM1") == 0) {
        serial_driver.driver_id = UDR_COM1_READLINE_SERVER;
        serial_driver.keyboard_id = UDR_DEV_COM1;
        serial_driver.com_port = COM1_IO_BASE;
    } else if (strcmp(com, "COM2") == 0) {
        serial_driver.driver_id = UDR_COM2_READLINE_SERVER;
        serial_driver.keyboard_id = UDR_DEV_COM2;
        serial_driver.com_port = COM2_IO_BASE;
    } else {
        printf("Bad com port provided for serial readline server");
        return -1;
    }
    if (ipc_server_init(&serial_driver.state, serial_driver.driver_id) < 0) {
        printf("could not register for readline server, exiting...\n");
        return -1;
    }
    return 0;
}

int main(int argc, char** argv)
{

    int pid;
    if ((pid = fork()) != 0) {
        if (pid < 0) {
            printf("serial readline server could not be started\n");
            return -1;
        } else {
            return 0;
        }
    }
    if (argc != 2) {
        return -1;
    }

    thr_init(4096);
    init_keyboard(&serial_driver.keyboard);

    if (setup_serial_driver(argv[1]) < 0) {
        return -1;
    }

    thr_create(interrupt_loop, NULL);

    while (1) {
        // receive a readline request
        driv_id_t sender;
        int len;
        if (ipc_server_recv(serial_driver.state, &sender, &len, sizeof(int), true) < 0) {
            printf("could not receive request, exiting...\n");
            ipc_server_cancel(serial_driver.state);
            return -1;
        }
        char buf[READLINE_MAX_LEN];
        int msg_len = handle_request(&serial_driver.keyboard, (char*)&buf, len, send_to_print);
        if (msg_len < 0) {
            request_msg_t req;
            req.cmd = COMMAND_CANCEL;
            udriv_send(sender, req.raw, sizeof(request_msg_t));
        } else {
            ipc_server_send_msg(serial_driver.state, sender, &buf, msg_len);
        }
    }
    // Should never get here
    return -1;
}
