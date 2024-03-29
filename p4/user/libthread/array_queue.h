/** @file array_queue.h
 *  @brief A set of macros for constructing a queue
 *  @author Evan Palmer (esp)
 **/
#ifndef QUEUE_H
#define QUEUE_H
#include <malloc.h>

#define QUEUE_DEFAULT_SIZE 8
#define QUEUE_MIN_RESIZE 8

/** @brief A struct for items needed for all queues */
struct queue {
    int deq;
    int size;
    int capacity;
    int data_size;
};

/** @brief Creates a queue type called "name" of items with type "type"
 *  @param name The name of the queue type
 *  @param type The type of items in the queueu
 **/
#define TYPEDEF_QUEUE(name, type) \
    typedef struct {              \
        struct queue descriptor;  \
        type* queue_data;         \
    } name

/* Macros for the different allowed queue_init arguments */
#define QUEUE_INIT_SELECT(_1,_2, NAME,...) NAME
#define QUEUE_INIT1(q) QUEUE_INIT2(q, QUEUE_DEFAULT_SIZE)
#define QUEUE_INIT2(q, start_size) ({ \
        queue_init_helper(&(q)->descriptor, \
                          (char **)&(q)->queue_data, \
                          sizeof(*(q)->queue_data) / sizeof(char),\
                          start_size); })

/** @brief Initalizes the datastructure created by TYPEDEF_QUEUE
 *
 *  @param q The queue to initialize
 *  @param start_size If present, the size of the queue
 *  @return void
 **/
#define QUEUE_INIT(...) QUEUE_INIT_SELECT(__VA_ARGS__, QUEUE_INIT2, QUEUE_INIT1)(__VA_ARGS__)

/** @brief Adds an item onto the queue
 *
 *  @param q The queue to add an item to
 *  @param item The item to add
 *  @return the time added
 **/
#define QUEUE_ADD(q, item) ({ \
    int index = queue_add_index(&(q)->descriptor, (char**)&(q)->queue_data); \
    (q)->queue_data[index] = (item); \
})

/** @brief Removes an item from the tail of the queue
 *
 *  Can only be called when queue is not empty, else behavior undefined
 *
 *  @param q The queue to remove from
 *  @return the popped item
 **/
#define QUEUE_REMOVE(q) ({ \
    int index = queue_remove_index(&(q)->descriptor, (char**)&(q)->queue_data); \
    (q)->queue_data[index]; \
})

/** @brief Peeks at the item which will next be removed from the queue
 *
 *  Can only be called when the queue is not empty, else behavior undefined
 *
 *  @param q The queue to peek at
 *  @return the item
 **/
#define QUEUE_PEEK(q) ({ \
    (q)->queue_data[queue_peek_index(&(q)->descriptor)]; })

/** @brief Is the queue currently empty
 *
 *  @param q The queue to check
 *  @return A nonzero int if the queue is empty
 **/
#define QUEUE_EMPTY(q) ({ \
        queue_empty_helper(&(q)->descriptor); })

/** @brief Gives the current size of the queue
 *
 * @param q The queue to get the size of
 * @return The size of the queue as an int
 **/
#define QUEUE_SIZE(q) ({ \
        queue_size_helper(&(q)->descriptor); })

#define QUEUE_FREE(q) free((q)->queue_data)

/* Functions called by the macros -- should not be used by user */
void queue_init_helper(struct queue* q, char** queue_data, int data_size, int start_size);
int queue_empty_helper(struct queue* q);
int queue_size_helper(struct queue* q);
int queue_peek_index(struct queue* q);
int queue_add_index(struct queue* q, char** queue_data);
int queue_remove_index(struct queue* q, char** queue_data);

#endif // QUEUE_H
