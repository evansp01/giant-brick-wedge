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
#include <elf_410.h>
#include <simics.h>
#include <cr.h>
#include <switch.h>
#include <stdint.h>
#include <seg.h>
#include <ureg.h>
#include <limits.h>
#include <eflags.h>
#include <stack_info.h>
#include <stdlib.h>
#include <mutex.h>
#include <asm.h>

/** @brief Address of the top of a kernel stack */
#define STACK_HIGH 0xFFFFFFF0
/** @brief Number of paramaters to the user main function */
#define NUM_PARAMS_TO_MAIN 5
/** @brief Size of a user stack */
#define USER_STACK_SIZE PAGE_SIZE
/** @brief Max size allowed for copying user input for exec */
#define EXEC_MAX_BYTES (4 * PAGE_SIZE)
/** @brief Magic unused stack address */
#define MAGIC_NUMBER 0xDEAD1337

static int user_exec(tcb_t* tcb, int flen, char* fname,
                     int argc, char** argv, int arglen);

/** @brief Get the length of all strings in the argv array
 *
 *  @param ppd The page directory of the current process
 *  @param argc The number of entries in argv
 *  @param argv The array of arguments
 *  @return The combined length of argv on success, less than zero on fail
 **/
int get_argv_length(ppd_t* ppd, int argc, char** argv)
{
    int i;
    int total_length = 0;
    for (i = 0; i < argc; i++) {
        int length = vm_user_strlen(ppd, argv[i], EXEC_MAX_BYTES);
        if (length < 0) {
            return -1;
        }
        total_length += length + 1;
        if(total_length > EXEC_MAX_BYTES){
            return -1;
        }
    }
    return total_length;
}

/** @brief A hander for the exec system call
 *
 *  @param state the state of userspace when exec was called
 *  @return void
 **/
void exec_syscall(ureg_t state)
{
    struct {
        char* fname;
        char** argv;
    } packet;

    tcb_t* tcb = get_tcb();
    if (get_thread_count(tcb->process) > 1) {
        state.eax = -1;
        return;
    }
    ppd_t* dir = tcb->process->directory;
    if (vm_read(dir, &packet, (void*)state.esi, sizeof(packet)) < 0) {
        state.eax = -1;
        return;
    }
    int argc, argvlen;
    int flen = vm_user_strlen(dir, packet.fname, EXEC_MAX_BYTES) + 1;
    if (flen <= 0) {
        state.eax = -1;
        return;
    }
    if ((argc = vm_user_arrlen(dir, packet.argv, EXEC_MAX_BYTES)) < 0) {
        state.eax = -1;
        return;
    }
    if ((argvlen = get_argv_length(dir, argc, packet.argv)) < 0) {
        state.eax = -1;
        return;
    }
    state.eax = user_exec(tcb, flen, packet.fname, argc, packet.argv, argvlen);
    return;
}

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @param user_esp The stack pointer of the user process
 *  @param user_eip The entry point to the user process
 *  @return The location of the created context on the kernel stack
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
    int filelen = exec2obj_userapp_TOC[i].execlen;
    // if not enough bytes, copy over as many as possible
    if (offset + size > filelen) {
        size = filelen - offset;
    }
    // Copy bytes over to buffer
    for (byte_index = 0; byte_index < size; byte_index++) {
        buf[byte_index] = exec2obj_userapp_TOC[i].execbytes[offset + byte_index];
    }
    return byte_index;
}

/** @brief Finds the min of an array of unsigned integers
 *
 *  @param array The array of integers
 *  @param len The length of the array
 *  @return The min
 **/
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

/** @brief Finds the max of an array of unsigned integers
 *
 *  @param array The array of integers
 *  @param len The length of the array
 *  @return The max
 **/
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

/** @brief load a process image into a page directory
 *
 *  This function does the best it can in a number of situations. Notably
 *  the sections .text and .rodata will be allocated as readonly only if they
 *  are not on the same page as data which needs to be allocated read/write
 *
 *  Similarly, .bss will be allocated as zfod only if it does not share a page
 *  with data which needs to be written
 *
 *  @param elf Struct containing elf file information
 *  @param dir Page directory to fill out
 *  @return Zero on success, less than zero on failure
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
    if (vm_alloc_readwrite(dir, (void*)min_start, max_end - min_start) < 0) {
        return -1;
    }
    //back the sections we will write to, leave the rest for zfod if possible
    if (vm_back(dir, elf->e_txtstart, elf->e_txtlen) < 0) {
        lprintf("text backing failed");
        return -1;
    }
    if (vm_back(dir, elf->e_datstart, elf->e_datlen) < 0) {
        lprintf("dat backing failed");
        return -1;
    }
    if (vm_back(dir, elf->e_rodatstart, elf->e_rodatlen) < 0) {
        lprintf("rodat backing failed");
        return -1;
    }
    getbytes(elf->e_fname, elf->e_txtoff,
             elf->e_txtlen, (char*)elf->e_txtstart);
    getbytes(elf->e_fname, elf->e_rodatoff,
             elf->e_rodatlen, (char*)elf->e_rodatstart);
    getbytes(elf->e_fname, elf->e_datoff,
             elf->e_datlen, (char*)elf->e_datstart);
    // set everything to readonly
    if (vm_set_readonly(dir, (void*)min_start, max_end - min_start) < 0) {
        return -1;
    }
    // set pages which need to be writeable to read/write this means that
    // if a readonly and write section are on the same page, both will be
    // writeable, so at least the program still runs
    if (vm_set_readwrite(dir, (void*)elf->e_datstart, elf->e_datlen) < 0) {
        return -1;
    }
    if (vm_set_readwrite(dir, (void*)elf->e_bssstart, elf->e_bsslen) < 0) {
        return -1;
    }
    return 0;
}

/** @brief Copy a string, and return the length copied
 *  @param dest The destination to copy the string to
 *  @param source The source to copy the string from
 *  @return The number of bytes copied
 **/
int strcpy_len(char* dest, char* source)
{
    int i = 0;
    while ((dest[i] = source[i]) != '\0') {
        i++;
    }
    return i + 1;
}

/** @brief Sets up the argv array above the beginning of the user stack space
 *
 *  @param argc The number of strings in argv
 *  @param argv An array of strings passed to main of the new program
 *  @param argvlen The number of characters in argv
 *  @return Pointer to the top of the new stack
 **/
uint32_t setup_argv(int argc, char** argv, int argvlen)
{
    int i;
    char* strings_start = (char*)(STACK_HIGH - argvlen);
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
 *  @param argc The number of strings in argv
 *  @param argv An array of strings passed to main of the new program
 *  @param argvlen The number of characters in argv
 *  @param stack_low The lowest address of the user stack
 *  @return Pointer to esp for the new stack
 **/
uint32_t setup_main_stack(int argc, char** argv,
                          int argvlen, uint32_t stack_low)
{
    uint32_t argv_start = setup_argv(argc, argv, argvlen);
    uint32_t* stack_current = (uint32_t*)argv_start;
    PUSH_STACK(stack_current, stack_low, uint32_t);
    PUSH_STACK(stack_current, STACK_HIGH, uint32_t);
    PUSH_STACK(stack_current, argv_start, uint32_t);
    PUSH_STACK(stack_current, argc, uint32_t);
    PUSH_STACK(stack_current, MAGIC_NUMBER, uint32_t);
    return (uint32_t)(stack_current);
}

/** @brief Calculates the required stack space for a given program
 *
 *  @param argvlen The total length of the argv array
 *  @param argc The number of strings in the argv array
 *  @return The required stack space
 **/
uint32_t stack_space(int argvlen, int argc)
{
    uint32_t space = USER_STACK_SIZE;
    space += argvlen * sizeof(char);
    space += argc * sizeof(char*);
    space += NUM_PARAMS_TO_MAIN * sizeof(uint32_t);
    return space;
}

/** @brief Allocate the stack for a new program
 *
 *  @param ppd The ppd of the process
 *  @param stack_low The lowest address of the stack to be allocated
 *  @return Zero on success, less than zero on failure
 **/
int allocate_stack(ppd_t* ppd, uint32_t stack_low)
{
    uint32_t stack_size = STACK_HIGH - stack_low + 1;
    if (vm_alloc_readwrite(ppd, (void*)stack_low, stack_size) < 0) {
        return -1;
    }
    // back only the first page of the user stack
    uint32_t stack_offset = stack_low + USER_STACK_SIZE - PAGE_SIZE;
    uint32_t stack_space = stack_size - USER_STACK_SIZE + PAGE_SIZE;
    return vm_back(ppd, stack_offset, stack_space);
}

/** @brief Popualte an elf structure with a program
 *
 *  @param elf The elf structure to populate
 *  @param fname The name of the program to populate the elf with
 *  @return Zero on success, less than zero on failure
 **/
int load_elf(simple_elf_t* elf, char* fname)
{
    if (elf_check_header(fname) < 0) {
        return -1;
    }
    elf_load_helper(elf, fname);
    return 0;
}

/** @brief Load a program into a process
 *
 *  @param tcb The tcb of the process to load the program into
 *  @param elf An elf file describing the program to load
 *  @param argc The number of strings in argv
 *  @param argv An array of strings passed to main of the new program
 *  @param arglen The number of characters in argv
 *  @return Zero on success, less than zero on failure
 **/
int load_process(tcb_t* tcb, simple_elf_t* elf,
                 int argc, char** argv, int arglen)
{
    pcb_t* pcb = tcb->process;
    if (create_proc_pagedir(elf, pcb->directory) < 0) {
        return -1;
    }
    uint32_t stack_low = STACK_HIGH - stack_space(arglen, argc);
    stack_low = page_align(stack_low);
    if (allocate_stack(pcb->directory, stack_low) < 0) {
        return -1;
    }
    uint32_t stack_entry = setup_main_stack(argc, argv, arglen, stack_low);
    // Craft kernel stack contents
    tcb->saved_esp = create_context(
        (uint32_t)tcb->kernel_stack, stack_entry, elf->e_entry);
    return 0;
}

/** @brief Create a process with a program loaded into it
 *
 *  Note: this call will panic on failure, and should only be used for required
 *  processes
 *
 *  @param fname The name of the program to load
 *  @param argc The number of strings in argv
 *  @param argv An array of strings passed to main of the new program
 *  @return The tcb of the created process
 **/
tcb_t* new_program(char* fname, int argc, char** argv)
{
    tcb_t* tcb = create_pcb_entry();
    if (tcb == NULL) {
        panic("cannot create tcb/pcb for new program");
    }
    int i, argspace = 0;
    for (i = 0; i < argc; i++) {
        argspace += strlen(argv[i]) + 1;
    }
    simple_elf_t elf;
    if (load_elf(&elf, fname) < 0) {
        panic("Cannot load elf for required program %s", fname);
    }
    pcb_t* pcb = tcb->process;
    pcb->directory = init_ppd();
    if (pcb->directory == NULL) {
        panic("Cannot create pcb for required program %s", fname);
    }
    switch_ppd(pcb->directory);
    if (load_process(tcb, &elf, argc, argv, argspace) < 0) {
        panic("Cannot exec required program %s", fname);
    }
    // Add the newly created thread to the thread list
    kernel_add_thread(tcb);
    // Register process for simics user space debugging
    sim_reg_process(tcb->process->directory->dir, fname);
    return tcb;
}

/** @brief Replace the directory in the pcb and update cr3
 *
 *  @param pcb Process whose directory will be replaced
 *  @param dir New directory
 *  @return void
 **/
void replace_pcb_dir(pcb_t *pcb, ppd_t* dir)
{
    disable_interrupts();
    pcb->directory = dir;
    switch_ppd(pcb->directory);
    enable_interrupts();
}

/** @brief Replace the current program in this process with another program
 *
 *  @param tcb The tcb of the process to replace programs in
 *  @param k_space The kernel space buffer with arguments to the new program
 *  @param argc The number of strings in argv
 *  @param k_argv An array of strings passed to main of the new program
 *  @param arglen The number of characters in argv
 *  @return Zero on success, less than zero on failure
 **/
int replace_process(tcb_t* tcb, void* k_space,
                    int argc, char** k_argv, int arglen)
{
    simple_elf_t elf;
    if (load_elf(&elf, k_space) < 0) {
        return -1;
    }
    pcb_t* pcb = tcb->process;
    ppd_t *new = init_ppd();
    if (new == NULL) {
        return -1;
    }
    //save the old directory in case we want to revert
    ppd_t* old_dir = pcb->directory;
    replace_pcb_dir(pcb, new);

    //switches to new directory;
    int status = load_process(tcb, &elf, argc, k_argv, arglen);
    if (status < 0) {
        // if we failed make sure to restore the old page directory
        ppd_t *tmp = pcb->directory;
        replace_pcb_dir(pcb, old_dir);
        free_ppd(tmp, pcb->directory);
    } else {
        // De-register the previously running process in simics
        sim_unreg_process(old_dir->dir);
        sim_reg_process(pcb->directory->dir, k_space);
        //if we succeeded free the old directory
        free_ppd(old_dir, pcb->directory);
    }
    return status;
}

/** @brief Exec from userspace, copies arguments to kernel space and replaces
 *         the current process
 *  @param tcb The tcb of the process to exec
 *  @param flen The length of the filename to exec
 *  @param fname The filename of the process to exec
 *  @param argc The number of strings in argv
 *  @param argv An array of strings passed to main of the new program
 *  @param arglen The number of characters in argv
 *  @return Zero on success, less than zero on failure
 **/
static int user_exec(tcb_t* tcb, int flen, char* fname,
                     int argc, char** argv, int arglen)
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
    int status = replace_process(tcb, k_space, argc, k_argv, arglen);
    free(k_space);
    return status;
}
