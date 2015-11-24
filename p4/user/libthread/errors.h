/** @file errors.h
 *
 *  @brief A few macros for handling errors
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef ERRORS_H
#define ERRORS_H

#include <syscall.h>
#include <stdio.h>
#include <simics.h>

#define EXIT_ERROR(error, ...) do { \
    lprintf(error, ## __VA_ARGS__); \
    printf(error, ## __VA_ARGS__); \
    task_vanish(-1); \
} while(0)

#define WARN(warning, ...) do { \
    lprintf(warning, ## __VA_ARGS__); \
} while(0)

#endif
