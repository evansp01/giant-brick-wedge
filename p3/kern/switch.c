/** @file switch.c
 *
 *  @brief C functions for context switching
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control.h>
#include <switch.h>
#include <simics.h>

/** @brief Retrieves the stack address of the thread to yield to
 *
 *  @param addr Stack address of 
 *  @param yield_tid Thread id of thread to yield to
 *  @return Zero on success, an integer less than zero on failure
 **/
int yield(void *addr, int yield_tid)
{
    extern kernel_state_t kernel_state;
    tcb_t *p_tcb = get_tcb_from_addr(addr);
    
    lprintf("Current tid: %d", p_tcb->id);
    lprintf("Current kernel stack: 0x%x", (int)p_tcb->kernel_stack);
    lprintf("Current page dir: 0x%x", (int)(p_tcb->parent)->directory);
    
    // TODO: The code to decide which thread to run if yield_tid == -1 should
    //       be moved to a scheduler once it has been written
    tcb_t *tcb;
    Q_FOREACH(tcb, &kernel_state.threads, all_threads) {
        // Ignore the calling thread
        if (tcb->id == p_tcb->id)
            continue;
        
        else {
            if ((yield_tid == -1)||(tcb->id == yield_tid)) {
                lprintf("Other tid: %d", tcb->id);
                switch_context(tcb->saved_esp, p_tcb);
                return 0;
            }
        }
    }
    return -1;
}