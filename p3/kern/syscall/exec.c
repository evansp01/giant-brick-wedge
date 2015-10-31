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
#include <ureg.h>
#include <mode_switch.h>
#include <limits.h>
#include <eflags.h>


#define STACK_HIGH 0xFFFFFFF0
#define USER_STACK_SIZE PAGE_SIZE

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return void
 **/
void* create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip)
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
    void* kernel_stack = (void*)stack;
    PUSH_STACK(kernel_stack, ureg, ureg_t);
    return kernel_stack;
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
int getbytes(const char* filename, int offset, int size, char* buf)
{
    int i, byte_index;
    for (i = 0; i < exec2obj_userapp_count; i++) {
        if (strcmp(filename, exec2obj_userapp_TOC[i].execname) == 0)
            break;
    }
    // No program matching the given filename found
    if (i == exec2obj_userapp_count){
        return -1;
    }
    // Check if given offset and size exceeds the file size
    if ((offset + size) > exec2obj_userapp_TOC[i].execlen){
        return -1;
    }
    // Copy bytes over to buffer
    for (byte_index = 0; byte_index < size; byte_index++) {
        buf[byte_index] = exec2obj_userapp_TOC[i].execbytes[offset + byte_index];
    }
    return byte_index;
}

uint32_t min(uint32_t array[], int len)
{
    int i;
    uint32_t min_val = UINT_MAX;
    for (i = 0; i < len; i++) {
        if (array[i] < min_val) {
            min_val = array[i];
        }
    }
    return min_val;
}

uint32_t max(uint32_t array[], int len)
{
    int i;
    uint32_t max_val = 0;
    for (i = 0; i < len; i++) {
        if (array[i] > max_val) {
            max_val = array[i];
        }
    }
    return max_val;
}

/** @brief Creates page directory and copies process data into memory
 *
 *  @param elf Struct containing elf file information
 *  @return page directory of new process
 **/
int create_proc_pagedir(simple_elf_t* elf, page_directory_t* dir)
{
    uint32_t starts[4] = { elf->e_txtstart, elf->e_rodatstart,
                           elf->e_datstart, elf->e_bssstart };
    uint32_t ends[4] = { elf->e_txtstart + elf->e_txtlen,
                         elf->e_rodatstart + elf->e_rodatlen,
                         elf->e_datstart + elf->e_datlen,
                         elf->e_bssstart + elf->e_bsslen };
    uint32_t min_start = min(starts, 4);
    uint32_t max_end = max(ends, 4);
    allocate_pages(dir, (void*)min_start, max_end - min_start, e_read_page);
    vm_make_writeable(dir, (void*)elf->e_datstart, elf->e_datlen);
    vm_make_writeable(dir, (void*)elf->e_bssstart, elf->e_bsslen);
    getbytes(elf->e_fname, elf->e_txtoff, elf->e_txtlen, (char*)elf->e_txtstart);
    getbytes(elf->e_fname, elf->e_rodatoff, elf->e_rodatlen, (char*)elf->e_rodatstart);
    getbytes(elf->e_fname, elf->e_datoff, elf->e_datlen, (char*)elf->e_datstart);
    return 0;
}

/** @brief Creates a new idle process
 *
 *  @return Pointer to tcb on success, null on failure
 **/
tcb_t* create_idle()
{
    tcb_t* tcb_entry = create_pcb_entry(NULL);

    if (load_program(tcb_entry, "fork_test1", 0, NULL) < 0) {
        return NULL;
    }
    return tcb_entry;
}

int vm_strspace(char* str)
{
    return strlen(str) + 1;
}

int strcpy_len(char* dest, char* source)
{
    int i = 0;
    while ((dest[i] = source[i]) != '\0') {
        i++;
    }
    return i + 1;
}

int get_argv_length(int argc, char** argv)
{
    int i;
    int total_length = 0;
    for (i = 0; i < argc; i++) {
        int length;
        if ((length = vm_strspace(argv[i])) < 0) {
            return -1;
        }
        total_length += length;
    }
    return total_length;
}

/** @brief Allocates the stack for the new process
 *
 *  @param cr2 Address of the page table
 *  @param stack_high Highest address for the new stack
 *  @param argc Number of user arguments
 *  @param argv Pointer to user arguments
 *  @return Pointer to the top of the new stack
 **/
uint32_t setup_argv(int argc, char** argv, int argv_total)
{
    int i;
    char* strings_start = (char*)(STACK_HIGH - argv_total);
    char* current_string = strings_start;
    char** pointers_start = (char**)(strings_start - argc);
    for (i = 0; i < argc; i++) {
        pointers_start[i] = current_string;
        int copied = strcpy_len(current_string, argv[i]);
        current_string += copied;
    }
    return (uint32_t)pointers_start;
}

/** @brief Sets up the stack for the new process
 *
 *  @param cr2 Address of the page table
 *  @param argc Number of user arguments
 *  @param argv Pointer to user arguments
 *  @return Pointer to esp for the new stack
 **/
uint32_t setup_main_stack(int argc, char** argv, int argv_total, uint32_t stack_low)
{
    uint32_t argv_start = setup_argv(argc, argv, argv_total);
    uint32_t* stack_current = (uint32_t*)argv_start;
    PUSH_STACK(stack_current, stack_low, uint32_t);
    PUSH_STACK(stack_current, STACK_HIGH, uint32_t);
    PUSH_STACK(stack_current, argv_start, uint32_t);
    PUSH_STACK(stack_current, argc, uint32_t);
    PUSH_STACK(stack_current, 0xDEAD1337, uint32_t);
    return (uint32_t)(stack_current - 1);
}

uint32_t argument_space(int argvlen, int argc)
{
    return argvlen * sizeof(char) + argc * sizeof(char*) + 5 * sizeof(uint32_t);
}

int allocate_stack(void* cr3, uint32_t stack_low)
{
    uint32_t stack_size = STACK_HIGH - stack_low;
    lprintf("start 0x%lx, size 0x%lx", stack_low, stack_size);
    return allocate_pages(cr3, (void*)stack_low, stack_size, e_write_page);
}

/** @brief Loads the given program file
 *
 *  @param pcb Process to load program on
 *  @param tcb Thread to load program on
 *  @param filename Name of the program to be loaded
 *  @return Pointer to tcb on success, null on failure
 **/
int load_program(tcb_t* tcb, char* filename, int argc, char** argv)
{
    simple_elf_t elf;
    if (elf_check_header(filename) < 0) {
        lprintf("%s not a process", filename);
        return -1;
    }
    elf_load_helper(&elf, filename);
    int argv_length = get_argv_length(argc, argv);
    if (argv_length < 0) {
        return -2;
    }
    pcb_t *pcb = tcb->parent;
    pcb->directory = create_page_directory();
    if (pcb->directory == NULL) {
        return -3;
    }
    uint32_t stack_low = STACK_HIGH -
        (argument_space(argv_length, argc) + USER_STACK_SIZE);
    set_cr3((uint32_t)pcb->directory);
    if (allocate_stack(pcb->directory, stack_low) < 0) {
        return -4;
    }
    if (create_proc_pagedir(&elf, pcb->directory) < 0) {
        return -4;
    }
    uint32_t stack_entry = setup_main_stack(argc, argv, argv_length, stack_low);
    // Craft kernel stack contents
    tcb->saved_esp = create_context(
            (uint32_t)tcb->kernel_stack, stack_entry, elf.e_entry);
    return 0;
}
