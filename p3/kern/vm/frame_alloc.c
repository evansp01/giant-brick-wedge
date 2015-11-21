/** @file frame_alloc.c
 *
 *  @brief Functions to allocate and free frames
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <stdio.h>
#include <page.h>
#include <common_kern.h>
#include <string.h>
#include <mutex.h>
#include "vm_internal.h"
#include <vm.h>

/** @brief Structure for the frame allocator */
static struct frame_alloc {
    int total_frames;
    int free_frames;
    int next_physical_frame;
    uint32_t* next_frame;
    void* zero_page;
    mutex_t lock;
} frames;

/** @brief Zero a frame
 *  @param frame The frame to zero
 *  @return void
 **/
void zero_frame(void* frame)
{
    ASSERT_PAGE_ALIGNED(frame);
    memset(frame, 0, PAGE_SIZE);
}

/** @brief Return the next physical frame not yet touched by the frame allocator
 *
 *  @return a frame or NULL if no more physical frames are available
 **/
static void* next_physical_frame()
{
    if (frames.next_physical_frame >= frames.total_frames) {
        return NULL;
    }
    int frame_index = frames.next_physical_frame;
    frames.next_physical_frame++;
    uint32_t frame = USER_MEM_START + PAGE_SIZE * frame_index;
    return (void*)frame;
}

void* get_zero_page()
{
    return frames.zero_page;
}

int user_frame_total()
{
    return frames.total_frames;
}

/** @brief Initializes the frame allocator
 *  @return void
 **/
void init_frame_alloc()
{
    frames.total_frames = machine_phys_frames() - USER_MEM_START / PAGE_SIZE;
    // the zfod frame doesn't count, since it can never be allocated
    frames.total_frames--;
    frames.free_frames = frames.total_frames;
    frames.next_physical_frame = 0;
    frames.next_frame = 0;
    frames.zero_page = next_physical_frame();
    zero_frame(frames.zero_page);
    mutex_init(&frames.lock);
}

/** @brief Allocates a frame
 *
 *  Allocates an unreserved frame
 *
 *  @return A pointer to the frame or NULL if there are no frames left
 **/
int alloc_frame(void* virtual, entry_t* table, entry_t model)
{
    mutex_lock(&frames.lock);
    // save the user attribute of the model
    int user_page = model.user;
    int zfod_page = model.zfod;
    // we want kernel mode while we map to prevent info leaks
    model.user = 0;
    model.zfod = 1;
    if (frames.next_frame != 0) {
        //allocate from implicit frame list
        *table = create_entry(frames.next_frame, model);
        // map it in kernel mode so we can zero it
        invalidate_page((void*)virtual);
        //retreive the implicit frame pointer
        frames.next_frame = *((uint32_t**)virtual);
    } else {
        void* physical = next_physical_frame();
        if (physical == NULL) {
            mutex_unlock(&frames.lock);
            return -1;
        }
        //allocate from physical frame list
        *table = create_entry(physical, model);
        // map it in kernel mode so we can zero it
        invalidate_page((void*)virtual);
    }
    frames.free_frames--;
    mutex_unlock(&frames.lock);
    zero_frame(virtual);
    // reset the user flag
    table->user = user_page;
    table->zfod = zfod_page;
    // now that the frame is zeroed others can read it
    invalidate_page((void*)virtual);
    return 0;
}

/** @brief Allocates a frame
 *
 *  Allocates an unreserved frame
 *
 *  @return A pointer to the frame or NULL if there are no frames left
 **/
int kernel_alloc_frame(entry_t* table, entry_t model)
{
    mutex_lock(&frames.lock);
    uint32_t* physical;
    if (frames.next_frame != 0) {
        //allocate from implicit frame list
        physical = frames.next_frame;
        *table = create_entry(physical, model);
        //retreive the implicit frame pointer
        frames.next_frame = (uint32_t*)*physical;
    } else {
        physical = next_physical_frame();
        if (physical == NULL) {
            mutex_unlock(&frames.lock);
            return -1;
        }
        //allocate from physical frame list
        *table = create_entry(physical, model);
    }
    frames.free_frames--;
    mutex_unlock(&frames.lock);
    zero_frame(physical);
    return 0;
}

/** @brief Frees an allocated frame allowing it to be reused
 *
 *  @param frame The frame to free
 *  @return void
 **/
void free_frame(void* virtual, void* physical)
{
    mutex_lock(&frames.lock);
    uint32_t** frame_ptr = virtual;
    //save the next frame pointer to this page using the current address
    *frame_ptr = frames.next_frame;
    //save the physical address to this frame as the next frame pointer
    frames.next_frame = physical;
    frames.free_frames++;
    mutex_unlock(&frames.lock);
}
