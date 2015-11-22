/** @file swexn.c
 *
 *  @brief Functions to handle swexn syscalls
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <control_block.h>
#include <stdlib.h>
#include <syscall_kern.h>
#include <eflags.h>
#include <seg.h>
#include <interrupt.h>


/** @brief Bits in the eflags register which can be set in user mode */
#define USER_FLAGS (EFL_OF|EFL_DF|EFL_SF|EFL_ZF|EFL_AF|EFL_PF|EFL_CF)

/** @brief Registers a software exception handler
 *  @return void
 */
void register_swexn(tcb_t *tcb, swexn_handler_t handler, void *arg, void *stack)
{
    tcb->swexn.handler = handler;
    tcb->swexn.arg = arg;
    tcb->swexn.stack = stack;
}

/** @brief Deregisters the software exception handler
 *  @return void
 */
void deregister_swexn(tcb_t *tcb)
{
    tcb->swexn.handler = NULL;
    tcb->swexn.arg = NULL;
    tcb->swexn.stack = NULL;
}

/** @brief Swexn handler
 *
 *  @param state Struct containing saved register state before exception
 *  @param tcb TCB of current thread
 *  @return void
 **/
void swexn_handler(ureg_t* state, tcb_t* tcb)
{
    // Copy then deregister current software exception handler
    swexn_t swexn = tcb->swexn;
    deregister_swexn(tcb);

    // Setup exception stack
    swexn_stack_t swexn_stack;
    swexn_stack.ret_addr = 0;
    swexn_stack.arg = swexn.arg;
    swexn_stack.ureg = (void *)((uint32_t)swexn.stack - sizeof(ureg_t));
    swexn_stack.state = *state;
    // clear the resume flag we give back to the user so it will match
    // the bit we see in the swexn system call
    swexn_stack.state.eflags = swexn_stack.state.eflags & ~(EFL_RF);
    void *start = (void *)((uint32_t)swexn.stack - sizeof(swexn_stack_t));
    vm_write_locked(tcb->process->directory, &swexn_stack, (uint32_t)start,
                    sizeof(swexn_stack_t));

    // Setup context to switch to exception handler
    void *new_esp = create_context((uint32_t)tcb->kernel_stack,
        (uint32_t)start, (uint32_t)swexn.handler);

    // Run software exception handler
    go_to_user_mode(new_esp);

    // Should never reach here
    panic("We are lost in the depths of swexn");
}

/** brief Are the user supplied eflags valid
 *
 *  @param user_eflags The user supplied eflags
 *  @param current_eflags The current user mode eflags
 *  @return A boolean integer
 **/
int eflags_valid(uint32_t user_eflags, uint32_t current_eflags) 
{
    uint32_t protected_flags = user_eflags & (~USER_FLAGS);
    uint32_t current_protected = current_eflags & (~USER_FLAGS);
    return protected_flags == current_protected;
}

/** @brief Checks the user provided arguments for swexn
 *  @param tcb TCB of current thread
 *  @param eip Address of first instruction of user exception handler
 *  @param esp User exception stack
 *  @param regs User provided return register state
 *  @param eflags The user provided eflags values
 *  @return 0 on success, a negative integer on failure
 */
int check_swexn(tcb_t *tcb, swexn_handler_t eip, void *esp, ureg_t *regs,
                uint32_t eflags)
{
    ppd_t *ppd = tcb->process->directory;

    // Error: Provided eip and/or esp3 are unreasonable
    if ((eip != 0)&&(esp != 0)) {
        // Check that eip is user read memory
        if (!vm_user_can_read(ppd, eip, sizeof(void*))) {
            return -1;
        }
        // Check that esp3 is in user write memory
        if (!vm_user_can_write(ppd, esp, sizeof(void*))) {
            return -1;
        }
    }

    // Error: Provided register values are unreasonable
    if (regs != NULL) {
        // Check that newureg is in user memory
        if (!vm_user_can_read(ppd, regs, sizeof(void*))) {
            return -1;
        }
        // Check ds/es/fs/gs/ss/cs
        if ((regs->ds != SEGSEL_USER_DS)||(regs->es != SEGSEL_USER_DS)||
            (regs->fs != SEGSEL_USER_DS)||(regs->gs != SEGSEL_USER_DS)||
            (regs->ss != SEGSEL_USER_DS)||(regs->cs != SEGSEL_USER_CS)) {
            return -1;
        }
        // Check that eip is in user read memory
        if (!vm_user_can_read(ppd, (void *)regs->eip, sizeof(void*))) {
            return -1;
        }
        // Check eflags
        if(!eflags_valid(eflags, regs->eflags)){
            return -1;
        }
        // Check that esp is in user write memory
        if (!vm_user_can_write(ppd, (void *)regs->esp, sizeof(void*))) {
            return -1;
        }
    }
    return 0;
}

/** @brief The swexn syscall
 *  @param state The current state in user mode
 *  @return void
 */
void swexn_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    ppd_t *ppd = tcb->process->directory;

    struct {
        void *esp3;
        swexn_handler_t eip;
        void *arg;
        ureg_t *newureg;
    } args;

    if(vm_read_locked(ppd, &args, state.esi, sizeof(args)) < 0){
        state.eax = -1;
        return;
    }
    mutex_lock(&ppd->lock);
    int ret = check_swexn(tcb, args.eip, args.esp3, args.newureg, state.eflags);
    mutex_unlock(&ppd->lock);
    // If either request cannot be carried out, syscall must fail
    if (ret < 0) {
        state.eax = ret;
        return;
    }

    // Deregister handler if one is registered
    if ((args.esp3 == 0)||(args.eip == 0)) {
        deregister_swexn(tcb);
    }

    // Register a new software exception handler
    else {
        uint32_t stack = (uint32_t)args.esp3 - sizeof(void *);
        register_swexn(tcb, args.eip, args.arg, (void *)stack);
    }

    // Not returning from system call, but going to user specified place
    if (args.newureg != NULL) {
        state = *(args.newureg);
    }
    // returning from the system call
    else {
        state.eax = 0;
    }
}
