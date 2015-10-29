


# tcb_t *get_tcb_to_scheudle(int tid);
# void *prepare_to_schedule(tcb_t *tcb);
# void restore_state(tcb_t *tcb);

.global yield
yield:
    mov 4(%esp), %edx
    push %edx
    call get_tcb_to_schedule
    mov %eax, %ecx
    mov %ecx, (%esp)
    call prepare_to_schedule
    sub $4, %esp
    # eax is the stack of the other dude
    pusha
    mov %eax, %esp
    mov %ecx, %eax
    popa
    push %eax
    call restore_state
    sub $4, %esp
    ret



#ret
#popa

/*
fork() {

   I'm the first thread

   tcb_t *tcb = make_new_tcb()
   stack = tcb->stack
   push_stuff_onto_stack {
      .eax = tcb;
   }
   tcb->stack_restore = &push_stuff_onto_stack
   add_tcb_to_whereveritgoes

   I'm still the first thread
}



struct context {
    registersssss
    thread_first_run_asm
}


context c = {
  .ecx = tcb
  .ret = user_mode_switch
}
*/

thread_first_run_asm:
   push %eax
   call thread_first_run

/*
#void thread_first_run(tcb_t *tcb) {



*/
