/** @file 410user/progs/idle.c
 *  @author ?
 *  @brief Idle program.
 *  @public yes
 *  @for p2 p3
 *  @covers
 *  @status done
 */

#include <syscall.h>
#include <simics.h>
 
int main()
{
    int pid = gettid();
	lprintf("my pid is: %d", pid);
    if (pid == gettid())
        lprintf("gettid works twice in a row");
    
    while (1) {
    }
}
