/* If you want to use assembly language instead of C,
 * delete this autostack.c and provide an autostack.S
 * instead.
 */

#include <syscall.h>
#include <malloc.h>
#include <simics.h>
#include <autostack.h>

#define EXCEPTION_STACK_SIZE PAGE_SIZE

struct autostack {
    unsigned int stack_high;
    unsigned int stack_low;
    void* handler_stack;
};

struct autostack stack;



void autostack_fault(void* arg, ureg_t* ureg)
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

void threaded_fault(void* arg, ureg_t* ureg)
{
    lprintf("Thread %d received an unhandled exception %x exiting",
            gettid(), ureg->cause);
    lprintf("Killing process");
    task_vanish(ureg->cause);
}

void install_autostack(void* stack_high, void* stack_low)
{
    stack.handler_stack = malloc(EXCEPTION_STACK_SIZE);
    stack.handler_stack = ((char*)stack.handler_stack) + EXCEPTION_STACK_SIZE;
    stack.stack_low = (unsigned int)stack_low;
    stack.stack_high = (unsigned int)stack_high;
    swexn(stack.handler_stack, autostack_fault, &stack, NULL);
}

void install_threaded()
{
    swexn(stack.handler_stack, threaded_fault, &stack, NULL);
}

void get_stack_bounds(void** stack_high, void** stack_low)
{
    *stack_high = (void*)stack.stack_high;
    *stack_low = (void*)stack.stack_low;
}
