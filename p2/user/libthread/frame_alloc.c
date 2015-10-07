#include <syscall.h>
#include <stdlib.h>
#include <simics.h>
#include <mutex.h>
#include <variable_queue.h>
#include <thr_internals.h>

typedef struct frame_node {
    Q_NEW_LINK(frame_node) link;
    void* frame;
    volatile int unused;
} frame_t;

Q_NEW_HEAD(frame_list_t, frame_node);

/** @brief A struct for keeping track of allocated frames */
struct frame_alloc {
    char* first_high;
    char* first_low;
    unsigned int frame_size;
    int num_frames;
    frame_list_t frames;
    mutex_t frame_mutex;
};

/** @brief Frame alloc struct */
static struct frame_alloc frame_info = {
    .first_high = NULL,
    .first_low = NULL,
    .frame_size = 0,
    .num_frames = 1
};

/** @brief Initialize the frame allocator with information about stack
 *
 *  Initializes the frame allocator with a size of frames to allocate and
 *  information about the stack of the initial thread. The initial thread's
 *  stack must not grow after this has been called. Also ensures that the
 *  stack of the calling thread is at least size
 *
 *  @param size The size of frames
 *  @param stack_high The start value of the stack for the main thread
 *  @param stack_low The end value of the stack for the main thread
 *  @return zero on success and less than zero on failure
 **/
int frame_alloc_init(unsigned int size, void* stack_high, void* stack_low)
{
    if (size == 0) {
        frame_info.frame_size = PAGE_SIZE;
    } else {
        frame_info.frame_size = (((size - 1) / PAGE_SIZE) + 1) * PAGE_SIZE;
    }
    frame_info.first_high = stack_high;
    frame_info.first_low = stack_low;
    char* required_low = frame_info.first_high - frame_info.frame_size;
    if (required_low < frame_info.first_low) {
        int count = (frame_info.first_low - required_low - 1) / PAGE_SIZE + 1;
        char *new_low = frame_info.first_low - PAGE_SIZE * count;
        int status = new_pages((void*)new_low, PAGE_SIZE * count);
        // Stack extension failed, let default exception handler run on retry
        if (status < 0) {
            return status;
        }
        // Update stack low address
        frame_info.first_low = new_low;
    }
    Q_INIT_HEAD(&frame_info.frames);
    mutex_init(&frame_info.frame_mutex);
    return 0;
}

/** @brief Determine the top of the stack based on the allocated page
 *
 *  @param page The start of the allocated region (lowest address)
 *  @return A pointer to the top of the stack
 **/
void* page_to_stack(void* page)
{
    if (page == frame_info.first_low) {
        return frame_info.first_high;
    }
    return ((char*)page) + frame_info.frame_size - sizeof(void*);
}

/** @brief Determine the pointer to the allocated page based on the stack
 *
 *  @param stack The top of the stack
 *  @return A pointer to the lowest allocated address
 **/
void* stack_to_page(void* stack)
{
    if (stack == frame_info.first_high) {
        return frame_info.first_low;
    }
    return ((char*)stack) - frame_info.frame_size + sizeof(void*);
}

/** @brief Get a pointer to the stack frame corresponding to index i
 *
 *  @param index The index of the stack frame to retreive
 *  @return A pointer to the stack frame
 **/
void* frame_ptr(int index)
{
    return frame_info.first_low - (frame_info.frame_size + PAGE_SIZE) * index;
}

/** @brief Gets the address of the stack which addr is on if the stack exists
 *  If the status returned is FIRST_STACK or THREAD_STACK, then *stack will be
 *  the highest address on the releveant stack. If the status is NOT_ON_STACK
 *  or UNALLOCATED_PAGE then the stack pointer will not be modified.
 *
 *  @param addr The address to determine the stack of
 *  @param stack A pointer which will be set to stack if a stack was found
 *  @return the status of the function
 **/
enum stack_status get_address_stack(void *addr, void** stack)
{
    char* esp = (char*)addr;
    char* max_esp = frame_info.first_high;
    char* min_esp = (char*)frame_ptr(frame_info.num_frames - 1);
    // if esp cannot be in a stack frame, return NULL
    if (esp > max_esp || esp < min_esp) {
        return NOT_ON_STACK;
    }
    // if esp is within the first stack frame
    if (esp <= frame_info.first_high && esp >= frame_info.first_low) {
        *stack = frame_info.first_high;
        return FIRST_STACK;
    }
    // so esp must be somewhere in the allocated thread stacks
    unsigned int offset = frame_info.first_low - esp;
    // which frame do we think it is in
    int candidate = (offset / (frame_info.frame_size + PAGE_SIZE)) + 1;
    char* candidate_low = frame_ptr(candidate);
    char* candidate_high = page_to_stack(candidate_low);
    if (esp >= candidate_low && esp <= candidate_high) {
        *stack = candidate_high;
        return THREAD_STACK;
    }
    //seems to be somewhere in an unallocated page
    return UNALLOCATED_PAGE;
}

/** @brief Create a new stack frame at alloc_page
 *
 *  @param alloc_page The location to allocate the stack at
 *  @return zero on success less than zero on failure
 **/
int alloc_address(void* alloc_page)
{
    int status = new_pages(alloc_page, frame_info.frame_size);
    if (status < 0) {
        lprintf("allocation at %p failed with status %d\n", alloc_page, status);
    }
    return status;
}

/** @brief Get an existing unused stack frame if such a frame exists
 *  @return an existing stack frame, or NULL if no such frame exists
 **/
void* get_existing_frame()
{
    frame_t* current;
    int found = 0;
    Q_FOREACH(current, &frame_info.frames, link)
    {
        if (current->unused == 1) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return NULL;
    }
    //return the frame that was found
    Q_REMOVE(&frame_info.frames, current, link);
    void* page = current->frame;
    free(current);
    return page;
}

/** @brief Allocate a new frame suitable for a thread stack
 *
 *  Creates a new stack frame for a thread. If no memory is available, this will
 *  return a null pointer
 *
 *  @return A pointer to the top of the new stack
 **/
void* alloc_frame()
{
    void* alloc_page, *stack_top;
    int status;
    mutex_lock(&frame_info.frame_mutex);
    //attempt to use a recycled frame
    if ((alloc_page = get_existing_frame()) != NULL) {
        stack_top = page_to_stack(alloc_page);
    } else {
        //no existing frames
        alloc_page = frame_ptr(frame_info.num_frames);
        status = alloc_address(alloc_page);
        if (status < 0) {
            stack_top = NULL;
        } else {
            frame_info.num_frames++;
            stack_top = page_to_stack(alloc_page);
        }
    }
    mutex_unlock(&frame_info.frame_mutex);
    return stack_top;
}

/** @brief Add a frame to the list of potentially available stack frames
 *
 *  @param page The address of the frame to add to the list
 *  @return a pointer to the created entry in the list of stack frames
 **/
frame_t* create_frame_entry(void* page)
{
    frame_t* node = (frame_t*)malloc(sizeof(frame_t));
    Q_INIT_ELEM(node, link);
    node->unused = 0;
    node->frame = page;
    Q_INSERT_TAIL(&frame_info.frames, node, link);
    return node;
}

/** @brief Free a previously allocated stack frame
 *
 *  Frees a previously allocated frame, allowing it to be reused
 *
 *  @param frame The frame pointer returned by alloc_frame
 *  @return void
 **/
void free_frame(void* stack)
{
    void* page = stack_to_page(stack);
    mutex_lock(&frame_info.frame_mutex);
    frame_t* node = create_frame_entry(page);
    node->unused = 1;
    mutex_unlock(&frame_info.frame_mutex);
}

/** @brief Free a previously allocated stack frame, and kills the current thread
 *
 *  Frees a previously allocated frame, allowing it to be reused
 *
 *  @param frame The frame pointer returned by alloc_frame
 *  @return Does not return
 **/
void free_frame_and_vanish(void* stack)
{
    void* page = stack_to_page(stack);
    mutex_lock(&frame_info.frame_mutex);
    frame_t* node = create_frame_entry(page);
    mutex_unlock(&frame_info.frame_mutex);
    free_and_vanish(&node->unused);
}
