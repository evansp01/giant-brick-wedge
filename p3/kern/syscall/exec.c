/** @file exec.c
 *  @brief Implementation of functions to load program via exec
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
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
#include <stack_info.h>
#include <stdlib.h>

#define STACK_HIGH 0xFFFFFFF0
#define USER_STACK_SIZE PAGE_SIZE

int get_argv_length(void* cr3, int argc, char** argv)
{
    int i;
    int total_length = 0;
    for (i = 0; i < argc; i++) {
        int length = vm_user_strlen(cr3, argv[i]);
        if (length < 0) {
            return -1;
        }
        total_length += length + 1;
    }
    return total_length;
}

void exec_syscall(ureg_t state)
{
    struct {
        char* fname;
        char** argv;
    } packet;

    tcb_t* tcb = get_tcb();
    if(get_thread_count(tcb->parent) > 1){
        goto return_fail;
    }
    ppd_t *dir = &tcb->parent->directory;
    // TODO: kill all other threads
    if (vm_read(dir, &packet, (void*)state.esi, sizeof(packet)) < 0) {
        goto return_fail;
    }
    int flen, argc, argvlen = 0;
    if ((flen = vm_user_strlen(dir, packet.fname)) < 0) {
        goto return_fail;
    } else {
        // add one for the null character
        flen += 1;
    }
    if ((argc = vm_user_arrlen(dir, packet.argv)) < 0) {
        goto return_fail;
    }
    if ((argvlen = get_argv_length(dir, argc, packet.argv)) < 0) {
        goto return_fail;
    }
    state.eax = user_exec(tcb, flen, packet.fname, argc, packet.argv, argvlen);
    return;
//node that we failed and return -- release mutex?
return_fail:
    state.eax = -1;
    return;
}


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
    if (i == exec2obj_userapp_count) {
        return -1;
    }
    // Check if given offset and size exceeds the file size
    if ((offset + size) > exec2obj_userapp_TOC[i].execlen) {
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
int create_proc_pagedir(simple_elf_t* elf, ppd_t* dir)
{
    uint32_t starts[4] = { elf->e_txtstart, elf->e_rodatstart,
                           elf->e_datstart, elf->e_bssstart };
    uint32_t ends[4] = { elf->e_txtstart + elf->e_txtlen,
                         elf->e_rodatstart + elf->e_rodatlen,
                         elf->e_datstart + elf->e_datlen,
                         elf->e_bssstart + elf->e_bsslen };
    uint32_t min_start = min(starts, 4);
    uint32_t max_end = max(ends, 4);
    //all pages should be writeable so we can write to them even
    vm_alloc_readwrite(dir, (void*)min_start, max_end - min_start);
    getbytes(elf->e_fname, elf->e_txtoff, 
            elf->e_txtlen, (char*)elf->e_txtstart);
    getbytes(elf->e_fname, elf->e_rodatoff,
            elf->e_rodatlen, (char*)elf->e_rodatstart);
    getbytes(elf->e_fname, elf->e_datoff,
            elf->e_datlen, (char*)elf->e_datstart);
    // set everything to readonly
    vm_set_readonly(dir, (void*)min_start, max_end - min_start);
    // set pages which need to be writeable to read/write this means that
    // if a readonly and write section are on the same page, both will be
    // writeable, so at least the program still runs
    vm_set_readwrite(dir, (void*)elf->e_datstart, elf->e_datlen);
    vm_set_readwrite(dir, (void*)elf->e_bssstart, elf->e_bsslen);
    return 0;
}

int strcpy_len(char* dest, char* source)
{
    int i = 0;
    while ((dest[i] = source[i]) != '\0') {
        i++;
    }
    return i + 1;
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
    char** pointers_start = ((char**)strings_start) - argc;
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
    return (uint32_t)(stack_current);
}

uint32_t stack_space(int argvlen, int argc)
{
    uint32_t space = USER_STACK_SIZE;
    space += argvlen * sizeof(char);
    space += argc * sizeof(char*);
    space += 5 * sizeof(uint32_t);
    return space;
}

int allocate_stack(ppd_t* ppd, uint32_t stack_low)
{
    uint32_t stack_size = STACK_HIGH - stack_low + 1;
    return vm_alloc_readwrite(ppd, (void*)stack_low, stack_size);
}

int exec(tcb_t* tcb, char* fname, int argc, char** argv, int argspace)
{
    simple_elf_t elf;
    if (elf_check_header(fname) < 0) {
        lprintf("%s not a process", fname);
        return -1;
    }
    elf_load_helper(&elf, fname);
    pcb_t* pcb = tcb->parent;
    if(init_ppd(&pcb->directory) < 0){
        return -3;
    }
    switch_ppd(&pcb->directory);
    if (create_proc_pagedir(&elf, &pcb->directory) < 0) {
        return -4;
    }
    uint32_t stack_low = STACK_HIGH - stack_space(argspace, argc);
    if (allocate_stack(&pcb->directory, stack_low) < 0) {
        return -4;
    }
    uint32_t stack_entry = setup_main_stack(argc, argv, argspace, stack_low);
    // Craft kernel stack contents
    tcb->saved_esp = create_context(
        (uint32_t)tcb->kernel_stack, stack_entry, elf.e_entry);
    return 0;
}

tcb_t* new_program(char* fname, int argc, char** argv)
{
    tcb_t* tcb_entry = create_pcb_entry();
    int i, argspace = 0;
    for (i = 0; i < argc; i++) {
        argspace += strlen(argv[i]) + 1;
    }
    if (exec(tcb_entry, fname, argc, argv, argspace) < 0) {
        pcb_t *proc = tcb_entry->parent;
        free_tcb(tcb_entry);
        free_pcb(proc);
        return NULL;
    }
    // Add the newly created thread to the thread list
    kernel_add_thread(tcb_entry);
    // Register process for simics user space debugging
    sim_reg_process(tcb_entry->parent->directory.dir, fname);
    
    return tcb_entry;
}

#define EXEC_MAX_BYTES (4 * PAGE_SIZE)

int user_exec(tcb_t* tcb, int flen, char* fname, int argc, char** argv, int arglen)
{
    size_t flen_space = flen * sizeof(char);
    size_t argv_space = argc * sizeof(char*);
    size_t string_space = arglen * sizeof(char);
    size_t total_space = flen_space + argv_space + string_space;
    if (total_space > EXEC_MAX_BYTES) {
        return -1;
    }
    char* k_space = malloc(total_space);
    if (k_space == NULL) {
        return -1;
    }
    char** k_argv = (char**)(k_space + flen);
    char* k_str_start = (char*)(k_argv + argc);
    memcpy(k_space, fname, flen * sizeof(char));
    memcpy(k_argv, argv, argc * sizeof(char*));
    int i;
    char* k_str_current = k_str_start;
    for (i = 0; i < argc; i++) {
        k_argv[i] = k_str_current;
        int copied = strcpy_len(k_str_current, argv[i]);
        k_str_current += copied;
    }
    //we have copied all the arguments to kernel space -- now try to exec
    ppd_t old_dir = tcb->parent->directory;
    int status = exec(tcb, k_space, argc, k_argv, arglen);
    if (status < 0) {
        // if we failed make sure to restore the old page directory
        tcb->parent->directory = old_dir;
        switch_ppd(&old_dir);
    } else {
        ppd_t *new_dir = &tcb->parent->directory;
        // De-register the previously running process in simics
        sim_unreg_process(old_dir.dir);
        sim_reg_process(new_dir->dir, k_space);
        //if we succeeded free the old directory
        free_ppd(&old_dir, new_dir);
    }
    free(k_space);
    return status;
}
