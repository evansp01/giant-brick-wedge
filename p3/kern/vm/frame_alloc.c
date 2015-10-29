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
#include <utilities.h>
#include <common.h>

/** @brief Structure for the frame allocator */
static struct frame_alloc {
    int total_frames;
    int reserved_frames;
    int free_frames;
    int next_physical_frame;
} frames;

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
    void* frame = LEA(USER_MEM_START, PAGE_SIZE, frame_index);
    ASSERT_PAGE_ALIGNED(frame);
    return frame;
}

/** @brief Initializes the frame allocator
 *  @return void
 **/
void init_frame_alloc()
{
    frames.total_frames = machine_phys_frames() - USER_MEM_START / PAGE_SIZE;
    frames.reserved_frames = 0;
    frames.free_frames = frames.total_frames;
    frames.next_physical_frame = 0;
}

/** @brief Reserves frames for later use
 *
 *  Reserves frame_count frames for later use. The sum of reserved frames and
 *  allocated frames will never exceed the number of physical frames on the
 *  system
 *
 *  @param frame_count The number of frames to reserver
 *  @return Zero on success, an integer less than zero on failure
 **/
int reserve_frames(int frame_count)
{
    if (frames.free_frames < frame_count) {
        return -1;
    }
    frames.free_frames -= frame_count;
    frames.reserved_frames += frame_count;
    return 0;
}

/** @brief Allocates a previously reserved frame
 *
 *  This function gets a previously reserved frame, decrementing the count
 *  of reserved frames by one. Frames are zeroed before being allocated
 *
 *  @return The pointer to the frame, or NULL if there are no reserved frames
 **/
void* get_reserved_frame()
{
    if (frames.reserved_frames == 0) {
        return NULL;
    }
    frames.reserved_frames--;
    void *frame = next_physical_frame();
    return frame;
}

/** @brief Allocates a frame
 *
 *  Allocates an unreserved frame
 *
 *  @return A pointer to the frame or NULL if there are no frames left
 **/
void* allocate_frame()
{
    if (reserve_frames(1) < 0) {
        return NULL;
    }
    return get_reserved_frame();
}

/** @brief Frees an allocated frame allowing it to be reused
 *
 *  @param frame The frame to free
 *  @return void
 **/
void free_frame(void* frame)
{
}

/** @brief Frees a number of reserved frames
 *
 *  Frees a number of reserved but unallocated frames allowing them to be
 *  reserved or allocated again
 *
 *  @param count The number of frames to free
 *  @return void
 **/
void free_reserved_frames(int count)
{
    if (frames.reserved_frames < count) {
        lprintf("Error freed too many reserved frames");
        frames.free_frames += frames.reserved_frames;
        frames.reserved_frames = 0;
        return;
    }
    frames.reserved_frames -= count;
    frames.free_frames += count;
}
