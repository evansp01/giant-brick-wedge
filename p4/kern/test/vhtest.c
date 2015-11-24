/** @file vhtest.c
 *  @brief Functions to test variable hash tables
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include "variable_htable.h"
#include <stdio.h>
#include <math.h>

/** @brief Structure for a list of items */
Q_NEW_HEAD(hash_list_t, item);
/** @brief Structure for a new hash table */
H_NEW_TABLE(hash_table_t, hash_list_t);

/** @brief Structure for hash table items */
typedef struct item {
    Q_NEW_LINK(item) links;
    int key;
    int value;
} item_t;

/** @brief Aborts the test upon error */
#define ABORT_ERROR(error) \
    do {                   \
        puts(error);       \
        return -1;         \
    } while (0)

/** @brief Global variance for the table */
static float variance = 0.0;
/** @brief Global mean for the table */
static float mean = 0;

/** @brief Number of hash table elements to test */
#define TEST_SIZE 1000000

/** @brief Calculates the variance
 *
 *  @param i Index of the bucket
 *  @param count Number of elements in the bucket
 *  @return Variance of the bucket
 **/
int calc_var(i, count) {
    variance += (count - mean)*(count - mean);
    return 0;
}

/** @brief Tests the variable hash tables
 *  @return 0 on success, a negative integer on failure
 **/
int main()
{
    hash_table_t table;
    if (H_INIT_TABLE(&table) < 0) {
        ABORT_ERROR("Table failed to allocate");
    }
    puts("Allocated successfully");
    int i;
    for (i = 0; i < TEST_SIZE; i++) {
        item_t* node = malloc(sizeof(item_t));
        node->key = i;
        node->value = i + 1;
        Q_INIT_ELEM(node, links);
        if (H_SIZE(&table) != i) {
            ABORT_ERROR("Size did not increase with insert");
        }
        if (H_SIZE(&table) - 1 > H_CAPACITY(&table) * MAX_LOAD_FACTOR) {
            ABORT_ERROR("Table did not resize according to load factor");
        }
        H_INSERT(&table, node, key, links);
    }
    puts("Insert and resize successfully");
    for (i = 0; i < TEST_SIZE; i++) {
        if (H_CONTAINS(&table, i, key, links)) {
            if (H_GET(&table, i, key, links)->value == i + 1) {
                if (H_REMOVE(&table, i, key, links)->value != i + 1) {
                    ABORT_ERROR("Remove element not the same as got");
                }
            } else {
                ABORT_ERROR("Value not as expected");
            }
        } else {
            ABORT_ERROR("Value not found");
        }
        if (H_CONTAINS(&table, i, key, links)) {
            ABORT_ERROR("Remove failed");
        }
        if (H_SIZE(&table) != TEST_SIZE - i - 1) {
            ABORT_ERROR("Size incorrect");
        }
    }
    puts("Remove get and contains worked");
    for (i = 0; i < TEST_SIZE; i++) {
        item_t* node = malloc(sizeof(item_t));
        node->key = i;
        node->value = i + 1;
        Q_INIT_ELEM(node, links);
        if (H_SIZE(&table) != i) {
            ABORT_ERROR("Size did not increase with insert");
        }
        if (H_SIZE(&table) - 1 > H_CAPACITY(&table) * MAX_LOAD_FACTOR) {
            ABORT_ERROR("Table did not resize according to load factor");
        }
        H_INSERT(&table, node, key, links);
    }
    mean = ((float)H_SIZE(&table)) / H_CAPACITY(&table);
    H_DEBUG_BUCKETS(&table, key, links, calc_var);
    printf("Mean bucket size %f  Stddev %f\n",
            mean, sqrt(variance/H_CAPACITY(&table)));
    for (i = 0; i < TEST_SIZE; i++) {
        item_t* node = malloc(sizeof(item_t));
        node->key = i;
        node->value = i + 2;
        Q_INIT_ELEM(node, links);
        if (H_SIZE(&table) != TEST_SIZE) {
            ABORT_ERROR("Table size changed when inserting present item");
        }
        if (H_SIZE(&table) - 1 > H_CAPACITY(&table) * MAX_LOAD_FACTOR) {
            ABORT_ERROR("Table did not resize according to load factor");
        }
        if(H_INSERT(&table, node, key, links)->value != i+1){
            ABORT_ERROR("Insert does not return previous element");
        }
    }
    puts("Insert of existing keys works");
    item_t *iter, *swap;
    int count = 0;
    H_FOREACH_SAFE(i, iter, swap, &table, links){
        count++;
    }
    if(!count == H_SIZE(&table)){
        ABORT_ERROR("foreach safe reached too few elements");
    } else {
        puts("Foreach safe reached the correct number of elements");
    }
    for (i = 0; i < TEST_SIZE; i++) {
        if (H_CONTAINS(&table, i, key, links)) {
            if (H_GET(&table, i, key, links)->value == i + 2) {
                if (H_REMOVE(&table, i, key, links)->value != i + 2) {
                    ABORT_ERROR("Remove element not the same as got");
                }
            } else {
                ABORT_ERROR("Value not as expected");
            }
        } else {
            ABORT_ERROR("Value not found");
        }
        if (H_CONTAINS(&table, i, key, links)) {
            ABORT_ERROR("Remove failed");
        }
        if (H_SIZE(&table) != TEST_SIZE - i - 1) {
            ABORT_ERROR("Size incorrect");
        }
    }
    puts("Updated keys removed with no problems");
    printf("Final htable capacity %d\n", H_CAPACITY(&table));
    return 0;
}
