
#ifndef USER_DRIVERS
#define USER_DRIVERS

#include <variable_queue.h>
#include <udriv_kern.h>

#define INTERRUPT_BUFFER_SIZE 512
#define CONTROL_NO_DEVICE 0

Q_NEW_HEAD(buf_list_t, int_buf);

/** @brief The struct for a device's queue of interrupts */
typedef struct int_buf {
    Q_NEW_LINK(int_buf) link;
    char buffer[INTERRUPT_BUFFER_SIZE];
    int producer;
    int consumer;
} int_buf_t;

/** @brief The struct which describes what an interrupt should do */
typedef struct int_control {
    driv_id_t device_index;
    buf_list_t listeners;
} int_control_t;

extern int_control_t interrupt_table[]; 


void init_user_drivers();

#endif // USER_DRIVERS
