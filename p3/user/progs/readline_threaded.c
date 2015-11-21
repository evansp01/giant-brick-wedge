/** @file user/progs/readline_threaded.c
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (esp)
 *  @brief Tests that readline does not interleave.
 *  @public no
 *  @for p3
 *  @covers fork() wait() vanish() readline()
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for getpid */
#include <stdlib.h>   /* for exit */
#include <simics.h>    /* for lprintf */
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("readline_threaded:");

/* Main */
int main() {
	int tid, status;
    char buf[100];
    report_start(START_CMPLT);
    
    tid = fork();
    
    if (tid < 0) {
        lprintf("cannot fork()");
        goto fail;
    }
    
    REPORT_FAILOUT_ON_ERR(magic_readline(100, buf));
    report_misc(buf);
    
    if (tid == 0) {
        tid = gettid();
        exit(tid);
    } else {
        if (wait(&status) != tid) {
            lprintf("wrong exit status returned");
            goto fail;
        }
        report_end(END_SUCCESS);
        exit(0);
    }
    
fail:
    report_end(END_FAIL);
    exit(1);
}
