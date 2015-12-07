
#ifndef USER_DRIVERS
#define USER_DRIVERS

#include <variable_queue.h>
#include <variable_htable.h>
#include <udriv_kern.h>

#define INTERRUPT_BUFFER_SIZE 512
#define CONTROL_NO_DEVICE 0

typedef unsigned long long message_t;

/** @brief The struct for list of devices/services */
Q_NEW_HEAD(device_list_t, device);

/** @brief Structure for hash table of devices/services */
H_NEW_TABLE(device_hash_t, device_list_t);

/** @brief The struct for contents of a single interrupt */
typedef struct interrupt {
    message_t msg;
    unsigned int size;
} interrupt_t;

/** @brief The struct for a device/server */
typedef struct device {
    Q_NEW_LINK(device) global;
    Q_NEW_LINK(device) interrupts;
    Q_NEW_LINK(device) tcb_link;
    driv_id_t driver_id;
    interrupt_t buffer[INTERRUPT_BUFFER_SIZE];
    int producer;
    int consumer;
    unsigned int port;
    unsigned int bytes;
    struct tcb *owner;
    const dev_spec_t* device_table_entry;
} device_t;

/** @brief The struct for hardware interrupts via IDT */
typedef struct int_control {
    // list of devices/servers mapped to the IDT entry
    device_list_t devices;
    int num_devices;
} int_control_t;

extern int_control_t interrupt_table[]; 

device_t *get_device(driv_id_t entry);
void init_user_drivers();

#endif // USER_DRIVERS
