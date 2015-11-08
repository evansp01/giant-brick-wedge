/** @file 410user/progs/wait_getpid.c
 *  @author mpa
 *  @brief Tests gettid()/wait()/fork()
 *  @public yes
 *  @for p3
 *  @covers gettid wait fork set_status vanish
 *  @status done
 */

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("wait_getpid:");

int main()
{
  int pid, status;

  report_start(START_CMPLT);

  pid = fork();

  if (pid < 0) {
    report_end(END_FAIL);
    exit(-1);
  }
  
  if (pid == 0) {
    pid = gettid();
    lprintf("child is exiting %d", pid);
    exit(pid);
    report_end(END_FAIL);
  }
  int ret;
  if ((ret = wait(&status)) != pid) {
    lprintf("parent wait returned wrong pid %d", ret);
    report_end(END_FAIL);
    exit(-1);
  }

  lprintf("parent wait returned right pid");
  
  if (status != pid) {
    report_end(END_FAIL);
    exit(-1);
  }

  report_end(END_SUCCESS);
  exit(0);
}
