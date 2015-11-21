/** @file debug_print.h
 *  @brief Interface printing to console and simics
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_DEBUG_PRINT_H
#define KERN_INC_DEBUG_PRINT_H

#include <simics.h>
#include <stdio.h>

// Uncomment to print debugging messages to the console
//#define DEBUG_PRINT

/** @brief Prints kernel messages to the console and simics **/
#define KPRINTF(...) do { \
    lprintf (__VA_ARGS__); \
    printf (__VA_ARGS__); \
} while (0)
    
#ifdef DEBUG_PRINT

/** @brief Prints debug messages to the console and simics **/
#define DPRINTF(...) do { \
    lprintf (__VA_ARGS__); \
    printf (__VA_ARGS__); \
} while (0)

#else

/** @brief Macro placeholder for debug printing **/    
#define DPRINTF(...)
    
#endif

#endif // KERN_INC_DEBUG_PRINT_H