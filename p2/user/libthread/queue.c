/** @file queue.c
 *  @brief Helper functions for a generic array based queue
 *
 *  A series of helper functions for implementing a queue. These functions
 *  are called by macros in queue.h, which allows for a strongly typed generic
 *  queue
 *
 *  @author Evan Palmer (esp)
 **/

#include <stdlib.h>
#include <string.h>
#include "queue.h"

/** @brief Initialize a queue struct, and the queue_data that goes with it
 *
 *  @param q The queue struct to initialize
 *  @param queue_data The queue data
 *  @param data_size The size of the datatype the queue is using
 *  @param start_size The initial size of the queue
 *  @return void
 **/
void queue_init_helper(struct queue* q, char** queue_data, int data_size, int start_size)
{
    q->deq = 0;
    q->size = 0;
    q->capacity = start_size;
    q->data_size = data_size;
    *queue_data = malloc(start_size * q->data_size * sizeof(char));
}

/** @brief The the location of an index in the queue
 *
 *  @param q The queue to use
 *  @param index The index to find in the queue
 *  @return The index inside the queue
 **/
inline int queue_index(struct queue* q, int index)
{
    return index % q->capacity;
}

/** @brief Is the queue empty
 *
 *  @param q The queue to check
 *  @return Is the queue empty
 **/
inline int queue_empty_helper(struct queue* q)
{
    return q->size == 0;
}

/** @brief Get the size of the queue
 *
 *  @param The queue to get the size of
 *  @return The size
 **/
inline int queue_size_helper(struct queue* q)
{
    return q->size;
}

/** @brief Grow the queue's data section to twice it's size
 *
 *  @param q The queue to grow
 *  @param queue_data A pointer to the queue's memory
 *  @return void
 **/
inline void queue_grow(struct queue* q, char** queue_data)
{
    int new_capacity = q->capacity * 2 + 10;
    *queue_data = realloc(*queue_data, new_capacity * q->data_size * sizeof(char));
    if (q->deq + q->size > q->capacity) {
        memcpy(*queue_data + (q->deq + new_capacity - q->capacity) * q->data_size,
               *queue_data + (q->deq) * q->data_size,
               (q->capacity - q->deq) * q->data_size * sizeof(char));
        q->deq += new_capacity - q->capacity;
    }
    q->capacity = new_capacity;
}

/** @brief shrinks the queue's data section to half it's size
 *
 *  The queue must be less than 1/4 full for this function to avoid losing data
 *
 *  @param q The queue to shrink
 *  @param queue_data A pointer to the queue's memory
 *  @return void
 **/
inline void queue_shrink(struct queue* q, char** queue_data)
{
    int new_capacity = q->capacity / 2;
    if (q->deq + q->size > q->capacity) {
        memcpy(*queue_data + (q->deq - (q->capacity - new_capacity)) * q->data_size,
               *queue_data + (q->deq) * q->data_size,
               (q->capacity - q->deq) * q->data_size * sizeof(char));
        q->deq -= (q->capacity - new_capacity);
    } else if (q->deq + q->size > new_capacity) {
        memcpy(*queue_data,
               *queue_data + (q->deq) * q->data_size,
               q->size * q->data_size * sizeof(char));

        q->deq = 0;
    }
    *queue_data = realloc(*queue_data, new_capacity * q->data_size * sizeof(char));
    q->capacity = new_capacity;
}

/** @brief Get the index of the queue's head
 *
 *  @param q The queue
 *  @return The index of the head
 **/
inline int queue_peek_index(struct queue* q)
{
    return q->deq;
}

/** @brief Get the index where an item should be added to the queue
 *
 *  This function does all that is required to add an item to the queue
 *  except instead of adding the item, it returns the index where the item
 *  should be added
 *
 *  @param q The queue
 *  @return The index to add the item at
 **/
int queue_add_index(struct queue* q, char** queue_data)
{
    if (q->size >= q->capacity) {
        queue_grow(q, queue_data);
    }
    int temp = queue_index(q, q->deq + q->size);
    q->size++;
    return temp;
}

/** @brief Remove an item from the queue, and give the item's location
 *
 *  Remove an item from the queue, and returns the index into queue_data where
 *  the removed item can be found
 *
 *  @param q The queue
 *  @return The index of the removed item
 **/
int queue_remove_index(struct queue* q, char** queue_data)
{
    if ((q->size + 10) * 4 < q->capacity) {
        queue_shrink(q, queue_data);
    }
    int temp = q->deq;
    q->deq = queue_index(q, temp + 1);
    //size becomes < 0 if too many pops happen
    q->size--;
    return temp;
}
