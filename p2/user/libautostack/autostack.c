/* If you want to use assembly language instead of C,
 * delete this autostack.c and provide an autostack.S
 * instead.
 */

#include <syscall.h>
#include <malloc.h>
#include <simics.h>
#include <autostack.h>

#define EXCEPTION_STACK_SIZE 1000 * sizeof(void*)

struct autostack {
    void* stack_high;
    void* stack_low;
    void* handler_stack;
};

struct autostack stack;

void autostack_fault(void* arg, ureg_t* ureg)
{
    // If not a pagefault, return and let default handler run
    if (ureg->cause != SWEXN_CAUSE_PAGEFAULT)
        return;
    
    // Allocate new page below current lowest point of stack
    // TODO: Should growth size be larger than a page size?
    // TODO: Calculate the required memory based on the faulting memory
    //       address, then do new_pages based on that size
    void* new_low = stack.stack_low - PAGE_SIZE;
    int status = new_pages(new_low, PAGE_SIZE);
    
    // Stack extension failed, let default exception handler run on retry
    if (status < 0) {
        lprintf("stack extension allocation at %p failed with status %d\n",
                new_low, status);
        return;
    }
    
    // Update stack low address
    else {
        stack.stack_low = new_low;
    }
    
    // Re-register exception handler with original register state
    swexn(stack.handler_stack, autostack_fault, &stack, ureg);
}

void threaded_fault(void* arg, ureg_t* ureg)
{
}

void install_autostack(void* stack_high, void* stack_low)
{
    stack.handler_stack = malloc(EXCEPTION_STACK_SIZE);
    stack.stack_low = stack_low;
    stack.stack_high = stack_high;
    swexn(stack.handler_stack, autostack_fault, &stack, NULL);
}

void install_threaded()
{
    swexn(stack.handler_stack, threaded_fault, &stack, NULL);
}

void get_stack_bounds(void **stack_high, void **stack_low) {
    *stack_high = stack.stack_high;
    *stack_low = stack.stack_low;
}

