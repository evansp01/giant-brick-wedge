/** @file test_queue.c
 *  @brief Tests for the generic queue implementation
 *  @author Evan Palmer (esp)
 **/

#include <stdio.h>
#include "queue.h"

TYPEDEF_QUEUE(int_queue, short);

/** @brief Add and remove num_items from q
 *
 *  @param q The queue being tested
 *  @param num_items The number of items to add and remove
 *  @return void
 **/
void test_queue(int_queue* q, int num_items)
{
    int i;
    for (i = 1; i <= num_items; i++) {
        queue_add(q, i);
    }
    for (i = 1; i <= num_items; i++) {
        int temp;
        if (queue_empty(q)) {
            printf("error empty but shouldn't be\n");
        }
        if ((temp = queue_remove(q)) != i) {
            printf("error expected %d got %d\n", i, temp);
        }
    }
    if (!queue_empty(q)) {
        printf("error queue not empty but should be\n");
    }
}

/** @brief Main of queue test, add and remove many items from the queue
 *  @return void
 **/
int main()
{
    int_queue q;
    queue_init(&q);
    int i;
    for (i = 10; i <= 10000; i += 10) {
        test_queue(&q, i);
    }
    return 0;
}
