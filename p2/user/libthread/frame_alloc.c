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

void frame_alloc_init(unsigned int size, void *stack_high, void *stack_low)
{
    frame_info.frame_size = ((size - 1 / PAGE_SIZE) + 1) * PAGE_SIZE;
    frame_info.first_high = stack_high;
    frame_info.first_low = stack_low;
}

void* alloc_frame()
{
    char* next_page = frame_info.first_low - (frame_info.frame_size + PAGE_SIZE) * frame_info.num_frames;
    if (new_pages(next_page, frame_info.frame_size) >= 0) {
        return next_page + frame_info.frame_size - sizeof(void*);
    } else {
        return NULL;
    }
    frame_info.num_frames++;
}


void free_frame(void *frame) {
    (void)frame;
}
