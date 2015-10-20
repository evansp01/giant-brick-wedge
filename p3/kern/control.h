
#ifndef CONTROL_H_
#define CONTROL_H_

#include <vm.h>
#include <datastructures/variable_queue.h>

typedef enum state {
    NOTYET,
    RUNNING,
    EXITED
} state_t;

Q_NEW_HEAD(pcb_list_t, pcb);
Q_NEW_HEAD(tcb_list_t, tcb);

typedef struct pcb {
   Q_NEW_LINK(pcb) all_processes;
   Q_NEW_LINK(pcb) siblings;
   /* scheduler lists */
   pcb_list_t children;
   tcb_list_t threads;
   int id;
   int parent_id;
   int reserved_pages;
   int exit_status;
   page_directory_t *directory;
   state_t state;
} pcb_t;

typedef struct tcb {
    Q_NEW_LINK(tcb) all_threads;
    Q_NEW_LINK(tcb) pcb_threads;
    /* scheduler lists */
    int id;
    int pid;
    void *kernel_stack;
    state_t state;
} tcb_t;

typedef struct kernel_state {
    pcb_list_t processes;
    tcb_list_t threads;
    int next_id;
} kernel_state_t;

//headers regarding create process
int init_kernel_state();
pcb_t *create_pcb_entry(pcb_t *parent_pcb);
tcb_t *create_tcb_entry(pcb_t *parent_pcb, void *stack);
void *allocate_kernel_stack();

int create_idle();
int load_program(pcb_t *pcb, tcb_t *tcb, char *filename);

#endif // CONTROL_H_