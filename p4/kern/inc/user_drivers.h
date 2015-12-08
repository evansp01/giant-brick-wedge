
#ifndef USER_DRIVERS
#define USER_DRIVERS

#include <variable_queue.h>
#include <variable_htable.h>
#include <udriv_kern.h>
#include <mutex.h>

#define INTERRUPT_BUFFER_SIZE 512
#define CONTROL_NO_DEVICE 0

typedef unsigned long long message_t;

/** @brief The struct for list of devices/servers */
Q_NEW_HEAD(devserv_list_t, devserv);

/** @brief Structure for hash table of devices/servers */
H_NEW_TABLE(device_hash_t, devserv_list_t);

/** @brief The struct for contents of a single interrupt */
typedef struct interrupt {
    driv_id_t driver_id;
    message_t msg;
    unsigned int size;
} interrupt_t;

/** @brief The struct for a device/server */
typedef struct devserv {
    Q_NEW_LINK(devserv) global;
    Q_NEW_LINK(devserv) interrupts;
    Q_NEW_LINK(devserv) tcb_link;
    driv_id_t driver_id;
    unsigned int port;
    unsigned int bytes;
    struct tcb *owner;
    const dev_spec_t *device_table_entry;
    mutex_t mutex;
} devserv_t;

/** @brief The struct for hardware interrupts via IDT */
typedef struct int_control {
    // list of devices/servers mapped to the IDT entry
    devserv_list_t devices;
    int num_devices;
} int_control_t;

extern int_control_t interrupt_table[]; 

int assign_driver_id();
int next_index_int(int index);
devserv_t *get_devserv(driv_id_t entry);
void add_devserv(devserv_t *device);
int check_add_devserv(devserv_t *device);
void remove_devserv(devserv_t *entry);
devserv_t *create_devserv_entry(driv_id_t id);
void free_devserv_entry(devserv_t *entry);
void init_user_drivers();

#endif // USER_DRIVERS
