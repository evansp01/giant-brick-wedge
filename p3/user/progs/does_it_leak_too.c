/** @file user/progs/does_it_leak_too.c
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (esp)
 *  @brief Test to check for memory leaks in failure instances of fork()
 *  @public no
 *  @for p3
 *  @covers new_pages() remove_pages() fork()
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for getpid */
#include <stdlib.h>   /* for exit */
#include <simics.h>    /* for lprintf */
#include "410_tests.h"
#include <report.h>

#define NUM_MEGABYTES 12
#define MEGABYTE (1024*1024)
#define SIZE (NUM_MEGABYTES*MEGABYTE)
#define NUM_ITER 10
#define SLEEP_TIME 15000

DEF_TEST_NAME("does_it_leak_too:");

/* touch one byte per page to run faster.
 * this code is a separate function so gcc doesn't optimize
 * away the array writing.
 */
void touch_array(char *p) {
    int i;
    for(i = SIZE - 1; i >= 0; i -= PAGE_SIZE) {
        p[i] = 'a';
    }
}

/* Main */
int main() {
    int i, count, ret_val, pid, num_fork, prev_num_fork;
    char local_array[SIZE];
    
    REPORT_START_CMPLT;
    touch_array(local_array);
    lprintf("touched it all");
    
    for (i=0; i<NUM_ITER; i++) {
        num_fork = 0;
        lprintf("counting forks for iteration: %d", i);
        while ((pid = fork()) >= 0) {
            if(pid == 0) {
                sleep(SLEEP_TIME);
                exit(0);
            } else {
                num_fork++;
            }
        }
        count = 0;
        while (count < num_fork) {
            wait(&ret_val);
            count++;
        }
        if (i != 0 && prev_num_fork != num_fork) {
            goto fail;
        }
        prev_num_fork = num_fork;   
        lprintf("iteration: %d, num_fork: %d", i, num_fork);
	}
    
    report_end(END_SUCCESS);
	exit(42);
    
fail:
    lprintf("YES IT DOES!");
    report_end(END_FAIL);
    exit(-1);
}