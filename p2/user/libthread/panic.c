/** @file panic.c
 *  @brief An implementation of the panic function
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs.
 **/

/* 
 * Copyright (c) 1996-1995 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <simics.h>
#include <syscall.h>

/** @brief General purpose panic function
 *
 *  Crashes the thread group when called
 *  Used by the assert() macro in assert.h
 *
 *  @param fmt Printout for panic function
 *  @return Void
 **/
void panic(const char *fmt, ...)
{
	va_list vl;
	char buf[80];

	va_start(vl, fmt);
	vsnprintf(buf, sizeof (buf), fmt, vl);
	va_end(vl);
	lprintf(buf);

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
	printf("\n");
    
    // exact authorship uncertain, popularized by Heinlein
    printf("When in danger or in doubt, run in circles, scream and shout.\n");
    lprintf("When in danger or in doubt, run in circles, scream and shout.");

    task_vanish(-1);
}
