/** @file user/progs/beauty_sleep.c
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (esp)
 *  @brief Tests that make_runnable cannot re-schedule sleeping threads
 *  @public no
 *  @for p3
 *  @covers fork() wait() vanish() sleep() make_runnable()
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for getpid */
#include <stdlib.h>   /* for exit */
#include <simics.h>    /* for lprintf */
#include "410_tests.h"
#include <report.h>

#define NUM_THREADS 10
#define NUM_TRIES 100

DEF_TEST_NAME("beauty_sleep:");

/* Main */
int main() {
    int ret_val, i, j;
    int pid = 0;
    int count = 0;    
    char *name = "sleep_test1";
    char *sleep_time = "10000";
    char *args[] = {name, sleep_time, 0};
    int pids[NUM_THREADS];
    
    REPORT_START_CMPLT;
    while(count < NUM_THREADS) {
		if((pid = fork()) == 0) {
            REPORT_ON_ERR(exec(name,args));
		}
		if(pid < 0) {
			goto fail;
		}
        pids[count] = pid;
        count++;
	}
    
    for (i=0; i<NUM_TRIES; i++) {
        for (j=0; j<NUM_THREADS; j++) {
            make_runnable(pids[j]);
        }
    }
    
    count = 0;
    while (count < NUM_THREADS) {
        wait(&ret_val);
        count++;
    }
    report_end(END_SUCCESS);
	exit(42);
    
fail:
    report_end(END_FAIL);
    exit(-1);
}