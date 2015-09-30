/* If you want to use assembly language instead of C,
 * delete this autostack.c and provide an autostack.S
 * instead.
 */

#include <syscall.h>
#include <malloc.h>
#include <simics.h>

#define EXCEPTION_STACK_SIZE 1000 * sizeof(void*)

struct autostack {
    void* stack_high;
    void* stack_low;
    void* handler_stack;
};

struct autostack stack;

void autostack_fault(void* arg, ureg_t* ureg)
{
}

void threaded_fault(void* arg, ureg_t* ureg)
{
}

void install_autostack(void* stack_high, void* stack_low)
{
    stack.handler_stack = _malloc(EXCEPTION_STACK_SIZE);
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

