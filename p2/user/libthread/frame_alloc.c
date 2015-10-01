#include <syscall.h>
#include <stdlib.h>
#include <simics.h>
#include <mutex.h>
#include <variable_queue.h>
#include <thr_internals.h>

typedef struct frame_node {
    Q_NEW_LINK(frame_node) link;
    void* frame;
    int unused;
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
 *  stack must not grow after this has been called
 *
 *  @param size The size of frames
 *  @param stack_high The start value of the stack for the main thread
 *  @param stack_low The end value of the stack for the main thread
 *  @return void
 **/
void frame_alloc_init(unsigned int size, void* stack_high, void* stack_low)
{
    if (size == 0) {
        frame_info.frame_size = PAGE_SIZE;
    } else {
        frame_info.frame_size = (((size - 1) / PAGE_SIZE) + 1) * PAGE_SIZE;
    }
    frame_info.first_high = stack_high;
    frame_info.first_low = stack_low;
    Q_INIT_HEAD(&frame_info.frames);
    mutex_init(&frame_info.frame_mutex);
}

void* page_to_stack(void* frame)
{
    return ((char*)frame) + frame_info.frame_size - sizeof(void*);
}

void* stack_to_page(void* stack)
{
    return ((char*)stack) - frame_info.frame_size + sizeof(void*);
}

void* frame_ptr(int index)
{
    return frame_info.first_low - (frame_info.frame_size + PAGE_SIZE) * index;
}

void* stack_start()
{
    char* esp = (char*)get_esp();
    char* max_esp = frame_info.first_high;
    char* min_esp = (char*)frame_ptr(frame_info.num_frames - 1);
    // if esp cannot be in a stack frame, return NULL
    // TODO: are these really < and >
    if (esp > max_esp || esp < min_esp) {
        return NULL;
    }
    // if esp is within the first stack frame
    // TODO: are these really <= and >=
    if (esp <= frame_info.first_high && esp >= frame_info.first_low) {
        return frame_info.first_high;
    }
    // so esp must be somewhere in the allocated thread stacks
    unsigned int offset = frame_info.first_low - esp;
    // which frame do we think it is in
    int canidate = (offset / (frame_info.frame_size + PAGE_SIZE)) + 1;

    char* canidate_low = frame_ptr(canidate);
    char* canidate_high = page_to_stack(canidate_low);

    // TODO: are these really <= and >=
    if (esp >= canidate_low && esp <= canidate_high) {
        return canidate_high;
    }
    //seems to be somewhere in an unallocated page
    return NULL;
}

int alloc_address(void* alloc_page)
{
    int status = new_pages(alloc_page, frame_info.frame_size);
    if (status < 0) {
        lprintf("allocation at %p failed with status %d\n", alloc_page, status);
    }
    return status;
}

void* get_existing_frame()
{
    frame_t* current;
    int found = 0;
    Q_FOREACH(current, &frame_info.frames, link)
    {
        if (!current->unused == 1) {
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

frame_t* create_frame_entry(void* page)
{
    frame_t* node = (frame_t*)malloc(sizeof(frame_t));
    Q_INIT_ELEM(node, link);
    node->unused = 0;
    node->frame = page;
    Q_INSERT_TAIL(&frame_info.frames, node, link);
    return node;
}


#define REUSE_FRAMES

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
#ifdef REUSE_FRAMES
    frame_t* node = create_frame_entry(page);
    node->unused = 1;
#endif
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
#ifdef REUSE_FRAMES
    frame_t* node = create_frame_entry(page);
    mutex_unlock(&frame_info.frame_mutex);
    free_and_vanish(&node->unused);
#else
    mutex_unlock(&frame_info.frame_mutex);
    vanish();
#endif
}
