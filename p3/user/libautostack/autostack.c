/** @file autostack.c
 *
 *  @brief Page fault handler for legacy stack growth, and a handler for
 *  normal threaded execution.
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall.h>
#include <malloc.h>
#include <simics.h>
#include <autostack.h>

#define EXCEPTION_STACK_SIZE PAGE_SIZE

/** @brief A struct for tracking stack growth */
struct autostack {
    unsigned int stack_high;
    unsigned int stack_low;
    void* handler_stack;
};

static struct autostack stack;


/** @brief Fault handler for legacy stack growth
 *
 *  @param arg A pointer we don't user
 *  @param ureg The register state and fault cause
 *  @return void
 **/
static void autostack_fault(void* arg, ureg_t* ureg)
{
    // If not a pagefault, return and let default handler run
    if (ureg->cause != SWEXN_CAUSE_PAGEFAULT) {
        //TODO: should we do something else here?
        return;
    }
    if ((ureg->error_code & 0x1) != 0) {
        //permissions error
        return;
    }
    if (ureg->cr2 > stack.stack_low)
        return;

    // Allocate new page below current lowest point of stack
    int page_count = (stack.stack_low - ureg->cr2 - 1) / PAGE_SIZE + 1;
    unsigned int new_low = stack.stack_low - PAGE_SIZE * page_count;
    int status = new_pages((void*)new_low, PAGE_SIZE * page_count);
    // Stack extension failed, let default exception handler run on retry
    if (status < 0) {
        lprintf("stack extension at %x failed with status %d", new_low, status);
        return;
    }
    // Update stack low address
    stack.stack_low = new_low;
    // Re-register exception handler with original register state
    swexn(stack.handler_stack, autostack_fault, &stack, ureg);
}

/** @brief Fault handler for threaded case. Kills task on exception
 *
 *  @param arg A pointer we don't user
 *  @param ureg The register state and fault cause
 *  @return void
 **/
static void threaded_fault(void* arg, ureg_t* ureg)
{
    lprintf("Thread %d received an unhandled exception 0x%x exiting",
            gettid(), ureg->cause);
    lprintf("Killing process");
    task_vanish(ureg->cause);
}

/** @brief Install the autostack page fault handler for legacy stack growth
 *
 *  @param stack_high The high address of main's stack
 *  @param stack_low The low address of main's stack
 *  @return void
 **/
void install_autostack(void* stack_high, void* stack_low)
{
    stack.handler_stack = malloc(EXCEPTION_STACK_SIZE);
    stack.handler_stack = ((char*)stack.handler_stack) + EXCEPTION_STACK_SIZE;
    stack.stack_low = (unsigned int)stack_low;
    stack.stack_high = (unsigned int)stack_high;
    swexn(stack.handler_stack, autostack_fault, &stack, NULL);
}

/** @brief Install page fault handler for threaded execution
 *
 *  We usually run this handler on the thread's main stack. This will
 *  corrupt the stack, however after this handler, the thread won't ever
 *  run again
 *
 *  @param thread_stack The stack to run the handler on
 *  @return void
 **/
void install_threaded(void *thread_stack)
{
    swexn(thread_stack, threaded_fault, &stack, NULL);
}

/** @brief Get the current bounds on the main thread's stack
 *
 *  This is only valid to call before thr_init has been called
 *
 *  @param stack_high This will be set to the high address of the main stack
 *  @param stack_low This will be set to the low address of the main stack
 *  @return void
 **/
void get_stack_bounds(void** stack_high, void** stack_low)
{
    *stack_high = (void*)stack.stack_high;
    *stack_low = (void*)stack.stack_low;
}
