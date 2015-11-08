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

/* --- Prototypes --- */

tcb_t *create_idle();
tcb_t *create_copy(tcb_t *tcb_parent, void *state);
int getbytes( const char *filename, int offset, int size, char *buf );
void *create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip);
int load_program(tcb_t* tcb, char* filename, int argc, char **argv);
int copy_program(pcb_t* pcb_parent, pcb_t* pcb_child);
void setup_for_switch(tcb_t *tcb);
int user_exec(tcb_t* tcb, int flen, char* fname, int argc, char** argv, int arglen);
tcb_t* new_program(char* fname, int argc, char** argv);

#endif /* _LOADER_H */
