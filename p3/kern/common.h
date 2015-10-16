#include <page_structs.h>
#include <datastructures/variable_queue.h>

typedef enum thread_state {
    THREAD_STATE,
} thread_state_t;


typedef enum process_state {
    PROCESS_STATE,
} process_state_t;

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
   int pages;
   int exit_status;
   page_directory_t *directory;
   process_state_t state;
} pcb_t;

typedef struct tcb {
    Q_NEW_LINK(tcb) all_threads;
    Q_NEW_LINK(tcb) pcb_threads;
    /* scheduler lists */
    int id;
    int pid;
    void *kernel_stack;
    thread_state_t state;
} tcb_t;

typedef struct kernel_state {
    pcb_list_t processes;
    tcb_list_t threads;
} kernel_state_t;


//headers regarding create process
int create_idle();
int init_kernel_state();
int init_pcb(pcb_t *pcb);
int init_tcb(tcb_t *tcb, void *stack);
void *allocate_kernel_stack();
int load_program(pcb_t *pcb, char *filename);
void create_context(pcb_t *pcb);
void restore_context(pcb_t *pcb);
void user_mode();

//fault handler stuff
void handle_kernel_fault();
void handle_user_fault();
void page_nonexistant();
void page_permissions();
void page_zfod();

//creating page directories
page_directory_t *init_kernel_vm();
page_directory_t *init_process_vm();

//writing to vm from kernel
int vm_copy_file(void *address, char* filename, int offset, int size);
int vm_copy_buf(void *address, void *buffer, int size);
int vm_zero(void *address, int size);

//page table manipulation
page_table_t *get_table(void *address, page_directory_t *directory);
void *get_page(void *address, page_table_t *table);
void zero_page_table(void *page);
void zero_frame(void *frame);
void copy_frame(void *frame, void *from);
