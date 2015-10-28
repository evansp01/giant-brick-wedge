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
    int ret = fork();
    lprintf("fork: %d", ret);
    
    int tid = gettid();
    lprintf("my tid is: %d", tid);
    
    while (1) {
        // uncomment this to see the scheduler at work!
        //lprintf("my tid is: %d", tid);
    }
}
