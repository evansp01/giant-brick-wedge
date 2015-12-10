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
#include <ipc_server.h>
#include <stdio.h>
#include <simics.h>
#include "readline_common.h"

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

keyboard_t keyboard;


void* interrupt_loop(void* arg)
{
    driv_id_t driv_recv;
    message_t scancode;
    unsigned int size;

    // register for keyboard driver
    if (udriv_register(UDR_KEYBOARD, KEYBOARD_PORT, 1) < 0) {
        printf("cannot register for keyboard driver");
        return (void*)-1;
    }

    while (true) {
        // get scancode
        if (udriv_wait(&driv_recv, &scancode, &size) < 0) {
            printf("user keyboard interrupt handler failed to get scancode");
            return (void*)-1;
        }
        if (driv_recv != UDR_KEYBOARD) {
            printf("received interrupt from unexpected source");
            return (void*)-1;
        }
        handle_scancode(&keyboard, (uint8_t)scancode, print);
    }
    return NULL;
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

    init_keyboard(&keyboard);

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
        char buf[READLINE_MAX_LEN];
        int msg_len = handle_request(&keyboard, (char*)&buf, len, print);
        if (msg_len < 0) {
            request_msg_t req;
            req.cmd = COMMAND_CANCEL;
            udriv_send(sender, req.raw, sizeof(request_msg_t));
        } else {
            ipc_server_send_msg(server_st, sender, &buf, msg_len);
        }
    }
    // Should never get here
    return -1;
}
