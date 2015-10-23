/** @file loader.c
 *  @brief Implementation of functions to load programs
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>
#include <simics.h>
#include <cr.h>
#include <switch.h>
#include <stdint.h>
#include <seg.h>
#include <eflags.h>
#include <ureg.h>
#include <mode_switch.h>

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return void
 **/
void create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip)
{
    //set_esp-1(stack);
    uint32_t eflags_start = EFL_RESV1 | EFL_IF;
    ureg_t ureg = {
        .ss = SEGSEL_USER_DS,
        .esp = user_esp,
        .eflags = eflags_start,
        .cs = SEGSEL_USER_CS,
        .eip = user_eip,
        .gs = SEGSEL_USER_DS,
        .fs = SEGSEL_USER_DS,
        .es = SEGSEL_USER_DS,
        .ds = SEGSEL_USER_DS
    };
    void *tcb = NULL;
    void *kernel_stack = (void*)stack;
    PUSH_STACK(kernel_stack, ureg, ureg_t);
    PUSH_STACK(kernel_stack, tcb, void*);
    user_mode_switch(kernel_stack);
}

/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on succes; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
    int i, byte_index;

    for (i = 0; i < exec2obj_userapp_count; i++) {
        if (strcmp(filename, exec2obj_userapp_TOC[i].execname) == 0)
            break;
    }

    // No program matching the given filename found
    if (i == exec2obj_userapp_count)
        return -1;

    // Check if given offset and size exceeds the file size
    if ((offset + size) > exec2obj_userapp_TOC[i].execlen)
        return -1;

    // Copy bytes over to buffer
    for (byte_index = 0; byte_index < size; byte_index++) {
        buf[byte_index] = exec2obj_userapp_TOC[i].execbytes[offset+byte_index];
    }

    return byte_index;
}

/** @brief Creates page directory and copies process data into memory
 *
 *  @param elf Struct containing elf file information
 *  @return page directory of new process
 **/
page_directory_t* create_proc_pagedir(simple_elf_t* elf)
{
    page_directory_t* dir = create_page_directory();
    if (dir == NULL) {
        return NULL;
    }
    set_cr3((uint32_t)dir);

    allocate_pages(dir, (void*)elf->e_txtstart, elf->e_txtlen, e_read_page);
    getbytes(elf->e_fname, elf->e_txtoff, elf->e_txtlen, (char*)elf->e_txtstart);

    allocate_pages(dir, (void*)elf->e_rodatstart, elf->e_rodatlen, e_read_page);
    getbytes(elf->e_fname, elf->e_rodatoff, elf->e_rodatlen, (char*)elf->e_rodatstart);

    allocate_pages(dir, (void*)elf->e_datstart, elf->e_datlen, e_write_page);
    getbytes(elf->e_fname, elf->e_datoff, elf->e_datlen, (char*)elf->e_datstart);

    allocate_pages(dir, (void*)elf->e_bssstart, elf->e_bsslen, e_write_page);
    memset((void*)elf->e_bssstart, 0, elf->e_bsslen);

    return dir;
}

/** @brief Creates a new idle process
 *
 *  @return Zero on success, an integer less than zero on failure
 **/
int create_idle()
{
    init_kernel_state();
    pcb_t* pcb_entry = create_pcb_entry(NULL);

    void* stack = allocate_kernel_stack();
    set_esp0((uint32_t)stack);
    if (stack == NULL)
        return -1;

    tcb_t* tcb_entry = create_tcb_entry(pcb_entry, stack);

    return load_program(pcb_entry, tcb_entry, "idle");
}

/** @brief Allocates the stack for the new process
 *
 *  @param cr2 Address of the page table
 *  @param stack_high Highest address for the new stack
 *  @param argc Number of user arguments
 *  @param argv Pointer to user arguments
 *  @return Pointer to the top of the new stack
 **/
uint32_t setup_argv(void *cr2, uint32_t stack_high, int argc, char** argv)
{
    uint32_t stack_low = 0xFFFFF000;
    uint32_t stack_size = stack_high - stack_low;
    allocate_pages(cr2, (void *)stack_low, stack_size, e_write_page);
    memset((void*)stack_low, 0, stack_size);
    return stack_high;
}

/** @brief Sets up the stack for the new process
 *
 *  @param cr2 Address of the page table
 *  @param argc Number of user arguments
 *  @param argv Pointer to user arguments
 *  @return Pointer to esp for the new stack
 **/
uint32_t setup_main_stack(void *cr2, int argc, char** argv)
{
    uint32_t stack_high = 0xFFFFFFFFF & STACK_ALIGN;
    uint32_t *stack_current = (uint32_t *)setup_argv(cr2, stack_high, argc, argv);
    uint32_t stack_low = 0xFFFFF000;
    stack_current[-1] = stack_low;
    stack_current[-2] = stack_high;
    stack_current[-3] = (uint32_t) stack_current;
    stack_current[-4] = argc;
    stack_current[-5] = 0xDEAD1337;
    return (uint32_t) (stack_current - 6);
}

/** @brief Loads the given program file
 *
 *  @param pcb Process to load program on
 *  @param tcb Thread to load program on
 *  @param filename Name of the program to be loaded
 *  @return Zero on success, an integer less than zero on failure
 **/
int load_program(pcb_t* pcb, tcb_t* tcb, char* filename)
{
    simple_elf_t elf;
    if (elf_check_header(filename) < 0) {
        lprintf("%s not a process", filename);
        return -1;
    }
    elf_load_helper(&elf, filename);
    pcb->directory = (page_directory_t *)create_proc_pagedir(&elf);
    if (pcb->directory == NULL) {
        return -2;
    }
    uint32_t stack_entry = setup_main_stack(pcb->directory, 0, NULL);
    // Craft kernel stack contents
    create_context((uint32_t)tcb->kernel_stack, stack_entry, elf.e_entry);

    return 0;
}
