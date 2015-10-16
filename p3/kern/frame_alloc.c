#include <stdio.h>

/** @brief Initializes the frame allocator
 *  @return void
 **/
void init_frame_alloc()
{
}

/** @brief Reserves frames for later use
 *
 *  Reservers frame_count frames for later use. The sum of reserved frames and
 *  allocated frames will never exceed the number of physical frames on the
 *  system
 *
 *  @param frame_count The number of frames to reserver
 *  @return Zero on success, an integer less than zero on failure
 **/
int reserve_frames(int frame_count)
{
}

/** @brief Allocates a previously reserved frame
 *
 *  This function gets a previously reserved frame, decrementing the count
 *  of reserved frames by one.
 *
 *  @return The pointer to the frame, or NULL if there are no reserved frames
 **/
void* get_reserved_frame()
{
}

/** @brief Allocates a frame
 *
 *  Allocates an unreserved frame
 *
 *  @return A pointer to the frame or NULL if there are no frames left
 **/
void* allocate_frame()
{
    if(reserve_frames(1) < 0){
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
}
