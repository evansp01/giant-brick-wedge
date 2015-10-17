/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 * Functions for the loading
 * of user programs from binary 
 * files should be written in
 * this file. The function 
 * elf_load_helper() is provided
 * for your use.
 */
/*@{*/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>
#include <simics.h>


/* --- Local function prototypes --- */ 


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

/*@}*/
