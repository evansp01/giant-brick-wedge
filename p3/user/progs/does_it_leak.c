/** @file user/progs/does_it_leak.c
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (esp)
 *  @brief Tests to check for memory leaks in new_pages/remove_pages
 *  @public no
 *  @for p3
 *  @covers new_pages() remove_pages()
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for getpid */
#include <stdlib.h>   /* for exit */
#include <simics.h>    /* for lprintf */
#include "410_tests.h"
#include <report.h>

#define NUM_ITER 10000

DEF_TEST_NAME("does_it_leak:");

/* Main */
int main() {
    int ret_val;
    int pid = 0;
    int count = 0;    
    char *name = "remove_pages_test1";
    char *args[] = {name, 0};
    
    REPORT_START_CMPLT;
    while(count < NUM_ITER) {
		if((pid = fork()) == 0) {
            REPORT_ON_ERR(exec(name,args));
		}
		if(pid < 0) {
			goto fail;
		}
        wait(&ret_val);
        lprintf("count: %d", count);
        count++;
	}
    report_end(END_SUCCESS);
	exit(42);
    
fail:
    lprintf("YES IT DOES!");
    report_end(END_FAIL);
    exit(-1);
}