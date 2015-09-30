#include <syscall.h>
#include <stdlib.h>
#include <simics.h>

struct frame_alloc {
    char* first_high;
    char* first_low;
    unsigned int frame_size;
    int num_frames;
};

static struct frame_alloc frame_info = {
    .first_high = NULL,
    .first_low = NULL,
    .frame_size = 0,
    .num_frames = 1
};

void frame_alloc_init(unsigned int size, void* stack_high, void* stack_low)
{
    if (size == 0) {
        frame_info.frame_size = PAGE_SIZE;
    } else {
        frame_info.frame_size = (((size - 1) / PAGE_SIZE) + 1) * PAGE_SIZE;
    }
    frame_info.first_high = stack_high;
    frame_info.first_low = stack_low;
}

void* alloc_frame()
{
    char* next_page = frame_info.first_low - (frame_info.frame_size + PAGE_SIZE) * frame_info.num_frames;
    frame_info.num_frames++;
    int status = new_pages(next_page, frame_info.frame_size);
    if (status >= 0) {
        return next_page + frame_info.frame_size - sizeof(void*);
    } else {
        lprintf("frame allocation at %p of size %d failed with error code %d and num_frames %d\n",
                next_page, frame_info.frame_size, status, frame_info.num_frames);
        return NULL;
    }
}

void free_frame(void* frame)
{
    (void)frame;
}
