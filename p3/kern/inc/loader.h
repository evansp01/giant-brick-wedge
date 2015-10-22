/** @file loader.h
 *  @brief Interface for functions to load programs
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef _LOADER_H
#define _LOADER_H

#include <stdint.h>
#include <vm.h>
#include <elf_410.h>
#include <control.h>

#define PUT_STACK(stack, value, type) \
    *((type*)(stack)) = (type)(value)

#define PUSH_STACK(stack, value, type)         \
    do {                                       \
        stack = (void*)(((type*)(stack)) - 1); \
        PUT_STACK(stack, value, type);         \
    } while (0)
        
#define STACK_ALIGN 0xFFFFFFF0

/* --- Prototypes --- */

int create_idle();
int getbytes( const char *filename, int offset, int size, char *buf );
void create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip);
page_directory_t* create_proc_pagedir(simple_elf_t* elf);
int load_program(pcb_t *pcb, tcb_t *tcb, char *filename);
uint32_t setup_argv(void *cr2, uint32_t stack_high, int argc, char** argv);
uint32_t setup_main_stack(void *cr2, int argc, char** argv);

#endif /* _LOADER_H */
