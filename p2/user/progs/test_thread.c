#include <syscall.h>
#include <simics.h>

int main()
{
    /* Test gettid() */
    lprintf("gettid(): %d", gettid());
    
    /* Test yield() */
    // lprintf("yield(-1): %d", yield(-1));
    
    /* Test deschedule() */
    // int x = 1;
    // lprintf("deschedule(&non-zero): %d", deschedule(&x));
    lprintf("deschedule(bad_pointer): %d", deschedule(0));
    
    /* Test make_runnable() */
    lprintf("make_runnable(scheduled): %d", make_runnable(gettid()));
    
    /* Test make_runnable() with deschedule() */
    // TODO: Need to test with separate threads
    // int y = 0;
    // lprintf("deschedule(&zero): %d", deschedule(&y));
    // lprintf("make_runnable(descheduled): %d", make_runnable(gettid()));
    
    /* Test get_ticks() */
    lprintf("get_ticks(): %u", get_ticks());
    
    /* Test sleep() */
    lprintf("sleep(-1): %d", sleep(-1));
    lprintf("get_ticks(): %u", get_ticks());
    lprintf("sleep(10): %d", sleep(10));
    lprintf("get_ticks(): %u", get_ticks());
    
    /* Test swexn() */
    // TODO: Add test for software exception register/deregister
    
    while(1){
        continue;
    }
}
