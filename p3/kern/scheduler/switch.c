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

//#define DEBUG 1

/** @brief Yields execution to another thread
 *
 *  @param yield_tid Thread id of thread to yield to
 *  @return Zero on success, an integer less than zero on failure
 **/
int yield(int yield_tid)
{
    extern kernel_state_t kernel_state;
    tcb_t *p_tcb = get_tcb();

#ifdef DEBUG
    lprintf("------------Yield------------");
    lprintf("Current tid: %d", p_tcb->id);
    lprintf("Current kernel stack: 0x%x", (int)p_tcb->kernel_stack);
    lprintf("Current page dir: 0x%x", (int)(p_tcb->parent)->directory);
#endif
    
    // TODO: The code to decide which thread to run if yield_tid == -1 should
    //       be moved to a scheduler once it has been written
    tcb_t *tcb;
    Q_FOREACH(tcb, &kernel_state.threads, all_threads) {
        // Ignore the calling thread
        if (tcb->id == p_tcb->id)
            continue;
        
        else {
            if ((yield_tid == -1)||(tcb->id == yield_tid)) {
#ifdef DEBUG
                lprintf("Other tid: %d", tcb->id);
                lprintf("Other saved esp: 0x%x", (int)tcb->saved_esp);
                lprintf("Other page dir: 0x%x", (int)(tcb->parent)->directory);
                lprintf("-----------------------------");
#endif
                switch_context(tcb->saved_esp, p_tcb);
                return 0;
            }
        }
    }
    return -1;
}

/** @brief Stores the saved_esp into the tcb
 *
 *  @param addr Stack address of 
 *  @param tcb Pointer to tcb
 *  @return void
 **/
void store_esp(void *saved_esp, tcb_t *tcb)
{
    tcb->saved_esp = saved_esp;
}
