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

/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );
void *create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip);
int user_exec(tcb_t* tcb, int flen, char* fname, int argc, char** argv, int arglen);
tcb_t* new_program(char* fname, int argc, char** argv);

#endif /* _LOADER_H */
