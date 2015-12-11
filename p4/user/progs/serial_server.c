/** @file serial_server.c
 *
 *  @brief Serial server to handle user readline requests
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
#define MAX_PRINT_LENGTH READLINE_MAX_LEN
#include "serial_console.h"

#define BUF_LEN 1024
#define COMMAND_CANCEL 1
#define MOD_CNTL_MASTER_INT 8
#define INTERRUPTS IER_RX_FULL_INT_EN

/** @brief The message type */
typedef union {
    message_t raw;
    struct {
        unsigned int sender;
        unsigned short len;
        unsigned char cmd;
        unsigned char byte;
    };
} request_msg_t;

/** @brief A structure containg driver ids for the serial driver */
struct {
    int suggest_id;
    driv_id_t read_id;
    driv_id_t print_id;
    driv_id_t keyboard_id;
    port_t com_port;
    keyboard_t keyboard;
    char read_buf[READLINE_MAX_LEN];
} serial_driver;

#define BAUD_RATE 115200

#define LSB(val) (val & 0xFF)
#define MSB(val) ((val >> 8) & 0xFF)
extern void console_set_server(driv_id_t serv);

/** @brief Write to a port region
 *  @param port The port to write to
 *  @param reg The register of the port region
 *  @param value The value to write
 **/
void write_port(port_t port, reg_t reg, int value)
{
    if (udriv_outb(port + reg, value) < 0) {
        lprintf("udriv_outb syscall failed");
    }
}

/** @brief A untility function which reads from a port region
 *  @param port The port region
 *  @param reg The register of the port region
 *  @return The value read
 **/
int read_port(port_t port, reg_t reg)
{
    unsigned char val;
    if (udriv_inb(port + reg, &val) < 0) {
        lprintf("udriv_inb syscall failed");
    }
    return val;
}

#define NEWLINE_SCANCODE 13
#define BACKSPACE_SCANCODE 8
/** @brief Turns a scancode from the serial driver into a character. Notably
 *         converts backspaces and newlines
 *  @param scan The scancode from the serial driver
 *  @return char The character read
 **/
char readchar(int scan)
{
    if (scan == NEWLINE_SCANCODE) {
        return '\n';
    }
    if (scan == BACKSPACE_SCANCODE) {
        return '\b';
    }
    return scan;
}

/** @brief Send all available characters to the serial driver
 *  @return void
 **/
void print_chars()
{
    char c;
    while (get_next_char(&c)) {
        write_port(serial_driver.com_port, REG_DATA, c);
    }
}

/** @brief Interrrupt loop for the serial port driver. Accepts characters
 *         from the serial port, and sends characters to be printed
 *  @param arg Unused
 *  @return -1 on failure
 **/
void* interrupt_loop(void* arg)
{
    driv_id_t driv_recv;
    message_t scancode;
    unsigned int size;

    // register for keyboard driver
    if (udriv_register(serial_driver.keyboard_id,
                       serial_driver.com_port + REG_DATA, 1) < 0) {
        printf("cannot register for com driver");
        return (void*)-1;
    }
    serial_driver.suggest_id = udriv_register(UDR_ASSIGN_REQUEST, 0, 0);
    if (serial_driver.suggest_id < 0) {
        printf("cannot register for print suggestion server");
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
        if (driv_recv == serial_driver.keyboard_id) {
            char c = readchar(scancode);
            handle_char(&serial_driver.keyboard, c, send_to_print);
            print_chars();
        } else if (driv_recv == serial_driver.suggest_id) {
            print_chars();
        } else {
            printf("received interrupt from unexpected source");
            return (void*)-1;
        }
    }
    return NULL;
}

/** @brief Choose a serial driver port based on the arguments to main
 *  @param com The communication port
 *  @return -1 on failure. Zero otherwise
 **/
int setup_serial_driver(char* com)
{
    if (strcmp(com, "COM1") == 0) {
        serial_driver.read_id = UDR_COM1_READLINE_SERVER;
        serial_driver.print_id = UDR_COM1_PRINT_SERVER;
        serial_driver.keyboard_id = UDR_DEV_COM1;
        serial_driver.com_port = COM1_IO_BASE;
    } else if (strcmp(com, "COM2") == 0) {
        serial_driver.read_id = UDR_COM2_READLINE_SERVER;
        serial_driver.print_id = UDR_COM2_PRINT_SERVER;
        serial_driver.keyboard_id = UDR_DEV_COM2;
        serial_driver.com_port = COM2_IO_BASE;
    } else if (strcmp(com, "COM3") == 0) {

        serial_driver.read_id = UDR_COM3_READLINE_SERVER;
        serial_driver.print_id = UDR_COM3_PRINT_SERVER;
        serial_driver.keyboard_id = UDR_DEV_COM3;
        serial_driver.com_port = COM3_IO_BASE;
    } else if (strcmp(com, "COM4") == 0) {
        serial_driver.read_id = UDR_COM4_READLINE_SERVER;
        serial_driver.print_id = UDR_COM4_PRINT_SERVER;
        serial_driver.keyboard_id = UDR_DEV_COM4;
        serial_driver.com_port = COM4_IO_BASE;
    } else {
        printf("Bad com port provided for serial readline server");
        return -1;
    }
    return 0;
}

/** @brief The main loop for the print server. Repeatedly services clients
 *         and sends their messages to the serial driver
 *
 *  @param arg Unused
 *  @return -1 on failure
 **/
void* print_server(void* arg)
{
    ipc_state_t* state;
    if (ipc_server_init(&state, serial_driver.print_id) < 0) {
        printf("could not register for print server, exiting...\n");
        return (void*)-1;
    }
    while (1) {
        // receive a readline request
        driv_id_t sender;
        int len = ipc_server_recv(state, &sender, printer.buf,
                                  MAX_PRINT_LENGTH, 1);
        if (len < 0) {
            printf("could not receive request, exiting...\n");
            ipc_server_cancel(state);
            return (void*)-1;
        }
        print_message(len, serial_driver.suggest_id);
        // print the message
    }
    return (void*)-1;
    // Should never get here
}

/** @brief Send the cliend a message indicating that readline failed
 *
 *  @param sender The client to respond to
 *  @return void
 **/
void respond_failure(driv_id_t sender)
{
    request_msg_t req;
    req.cmd = COMMAND_CANCEL;
    udriv_send(sender, req.raw, sizeof(request_msg_t));
}

/** @brief The main loop for the readline server. Repeatedly services clients
 *         getting lines from the serial driver
 *
 *  @return -1 on failure
 **/
int readline_server()
{
    ipc_state_t* state;
    if (ipc_server_init(&state, serial_driver.read_id) < 0) {
        printf("could not register for readline server, exiting...\n");
        return -1;
    }

    while (1) {
        // receive a readline request
        driv_id_t sender;
        int len;
        int bytes = ipc_server_recv(state, &sender, &len, sizeof(int), 1);
        if (bytes < 0) {
            printf("could not receive request, exiting...\n");
            ipc_server_cancel(state);
            return -1;
        }
        // dude better send us four bytes
        if (bytes != sizeof(int)) {
            respond_failure(sender);
            continue;
        }
        int msg_len = handle_request(&serial_driver.keyboard,
                                     serial_driver.read_buf,
                                     len, send_to_print);
        if (msg_len < 0) {
            respond_failure(sender);
            continue;
        }
        ipc_server_send_msg(state, sender, &serial_driver.read_buf, msg_len);
    }
    // Should never get here
    return -1;
}

/** @brief Entrypoint of program. Forks and starts several servers
 *  @param argc The number of parameters passed by exec
 *  @param argv The arguments to the program.
 *  @return -1 on failure
 **/
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
    init_console();
    init_keyboard(&serial_driver.keyboard);

    if (setup_serial_driver(argv[1]) < 0) {
        return -1;
    }

    thr_create(interrupt_loop, NULL);
    thr_create(print_server, NULL);
    return readline_server();
}
