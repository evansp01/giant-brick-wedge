/** @file user/progs/print_threaded.c
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (esp)
 *  @brief Tests that print does not interleave.
 *  @public no
 *  @for p3
 *  @covers fork() wait() vanish() print()
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for getpid */
#include <stdlib.h>   /* for exit */
#include <simics.h>    /* for lprintf */
#include "410_tests.h"
#include <report.h>

#define NUM_PRINT 10

DEF_TEST_NAME("print_threaded:");

/* Main */
int main() {
	int i, tid, status;
    report_start(START_CMPLT);
    
    tid = fork();
    
    if (tid < 0) {
        lprintf("cannot fork()");
        goto fail;
    }
    for (i = 0; i < NUM_PRINT; i++) {
        if (print(13, "Hello World!\n") != 0) {
            lprintf("failed to print");
            goto fail;
        }           
    }
    
    if (tid > 0) {
        if (wait(&status) != tid) {
            lprintf("wrong exit status returned");
            goto fail;
        }    
        report_end(END_SUCCESS);
        exit(0);
    } else {
        tid = gettid();
        exit(tid);
    }
    
fail:
    report_end(END_FAIL);
    exit(1);
}