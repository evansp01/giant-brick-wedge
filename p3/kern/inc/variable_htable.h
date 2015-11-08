/** @file variable_htable.h
 *
 * @brief Generalized hash table based on variable queue
 *
 * @author Evan Palmer (esp)
 **/

#include <stdint.h>
#include <stdlib.h>
#include "variable_queue.h"

/* User defined hash function */
uint32_t hash_int(uint32_t x);

/** @brief The maximum ratio of size to capacity before a grow */
#define MAX_LOAD_FACTOR 6
/** @brief The maximum ratio of capcity to size before a shrink */
#define MIN_LOAD_FACTOR 4
/** @brief The initial size of the hash table */
#define INITIAL_CAPACITY 8

/** @def H_NEW_TABLE(H_TABLE_TYPE, Q_HEAD_TYPE)
 *
 *  @brief Generates a new structure of H_TABLE_TYPE representing a hash table
 *  with buckets of Q_HEAD_TYPE
 *
 *  @param H_TABLE_TYPE the type of the newly genereated hash table struct
 *  @param Q_HEAD_TYPE the type of the head node of the tables buckets
 **/
#define H_NEW_TABLE(H_TABLE_TYPE, Q_HEAD_TYPE) \
    typedef struct {                           \
        int current_capacity;                  \
        int current_size;                      \
        Q_HEAD_TYPE* htable;                   \
    } H_TABLE_TYPE

/** @def H_INIT_TABLE(table)
 *
 *  @brief Initializes a new hash table
 *  @param table The hash table to initialize
 *  @return 0 if the initialization was successful less than zero otherwise
 **/
#define H_INIT_TABLE(table)                                             \
    ({                                                                  \
        _H_SIZE(table) = 0;                                             \
        _H_CAP(table) = INITIAL_CAPACITY;                               \
        _H_TABLE(table) = _H_ALLOC_TABLE(table, INITIAL_CAPACITY);      \
        ((-1)*(_H_TABLE(table) == NULL));                               \
    })

/** @def H_FREE_TABLE(table)
 *
 *  @brief Frees the memory associated witha a hash table
 *  @param Table the table to free
 *  @return void
 **/
#define H_FREE_TABLE(table) \
        free(_H_TABLE(table))

/** @def H_EMPTY(table)
 *
 *  @brief Is this hash table currently empty
 *  @param The hash table to check
 *  @return 1 if the hash table size is empty 0 otherwise
 **/
#define H_EMPTY(table) \
    (_H_SIZE(table) == 0)

/** @def H_SIZE(table)
 *
 *  @brief Get the size of the hash table
 *  @param table The hash table to get the size of
 *  @return The size of the hash table
 **/
#define H_SIZE(table) \
    ({ _H_SIZE(table); })

/** @def H_CAPACITY(table)
 *
 * @brief Get the current number of buckets in the hash table.
 *
 * This is not a limit on how many things may be inserted. Only useful to
 * determine the current number of buckets
 *
 * @param table The table to inspect
 * @return The number of buckets
 **/
#define H_CAPACITY(table) \
    ({ _H_CAP(table); })


/** @def H_INSERT(table, elem, key_field, link_name)
 *
 *  @brief Insert an element into the hash table
 *
 *  @param table The hash table to insert into
 *  @param elem The element to insert
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @return The element previously at this position or NULL
 **/
#define H_INSERT(table, elem, key_field, link_name)                          \
    ({                                                                       \
        typeof(*_H_TABLE(table))* _bucket;                                   \
        typeof(*Q_GET_FRONT(_bucket))* _temp;                                \
        _H_GROW(table, key_field, link_name);                                \
        _bucket = _H_BUCKET(table, _H_HASH(table, _H_KEY(elem, key_field))); \
        _temp = _H_BUCKET_REMOVE(_bucket, _H_KEY(elem, key_field),           \
                                key_field, link_name);                       \
        _H_BUCKET_INSERT(_bucket, elem, link_name);                          \
        if(_temp == NULL) {                                                  \
            _H_SIZE(table)++;                                                \
        }                                                                    \
        _temp;                                                               \
    })

/** @def H_REMOVE(table, key, key_field, link_name)
 *
 *  @brief Removes the element with a matching key if such an element exists
 *
 *  @param table The table to remove from
 *  @param key The key of the element to remove
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @return The node which was removed or NULL
 */
#define H_REMOVE(table, key, key_field, link_name)                        \
    ({                                                                    \
        typeof(*_H_TABLE(table))* _bucket;                                \
        typeof(*Q_GET_FRONT(_bucket))* _temp;                             \
        _bucket = _H_BUCKET(table, _H_HASH(table, key));                  \
        _temp = _H_BUCKET_REMOVE(_bucket, key, key_field, link_name);     \
        if(_temp != NULL) {                                               \
            _H_SIZE(table)--;                                             \
        }                                                                 \
        _H_SHRINK(table, key_field, link_name);                           \
        _temp;                                                            \
    })

/** @def H_CONTAINS(table, key, key_field, link_name)
 *
 *  @brief Determines if the hash table contains an element with this key
 *
 *  @param table The table to check
 *  @param key The key to find
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @return 1 if the hash table contains such an item 0 else
 **/
#define H_CONTAINS(table, key, key_field, link_name)                \
    ({                                                              \
        typeof(*_H_TABLE(table))* _bucket;                          \
        _bucket = _H_BUCKET(table, _H_HASH(table, key));            \
        _H_BUCKET_GET(_bucket, key, key_field, link_name) != NULL;  \
    })

/** @def H_GET(table, key, key_field, link_name)
 *
 *  @brief Gets the element with the specified key if one exists
 *
 *  @param table They table to get from
 *  @param key The key of the element to get
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @return The element found or NULL
 **/
#define H_GET(table, key, key_field, link_name)                   \
    ({                                                            \
        typeof(*_H_TABLE(table))* _bucket;                        \
        _bucket = _H_BUCKET(table, _H_HASH(table, key));          \
        _H_BUCKET_GET(_bucket, key, key_field, link_name);        \
    })

/** @def H_DEBUG_BUCKETS(table, key_field, link, info)
 *
 *  @brief A debugging function that allows the user some access to the hash
 *         tables internals
 *
 *  @param table The table to get information about
 *  @param key_field The field where the key is stored in the element
 *  @param link The name of the link used to organize the bucket lists
 *  @param info A funciton accepting two integers which will be called with the
 *         index and size of every bucket
 *  @return void
 **/
#define H_DEBUG_BUCKETS(table, key_field, link, info)   \
    do {                                                \
        int _i;                                         \
        int _count;                                     \
        typeof(*_H_TABLE(table))* _bucket;              \
        typeof(*Q_GET_FRONT(_bucket)) *_search;         \
        for (_i = 0; _i < _H_CAP(table); _i++) {        \
            _bucket = _H_BUCKET(table, _i);             \
            _count = 0;                                 \
            Q_FOREACH(_search, _bucket, link) {         \
                _count++;                               \
            }                                           \
            info(_i, _count);                           \
        }                                               \
    } while(0)


/** @def H_DEBUG_BUCKETS(table, key_field, link, info)
 *
 *  @brief A debugging function that allows the user some access to the hash
 *         tables internals
 *
 *  @param table The table to get information about
 *  @param key_field The field where the key is stored in the element
 *  @param link The name of the link used to organize the bucket lists
 *  @param info A funciton accepting two integers which will be called with the
 *         index and size of every bucket
 *  @return void
 **/
#define H_FOREACH_SAFE(i, current, swap, table, link)               \
    for (i = 0; i < _H_CAP(table); i++)                             \
        Q_FOREACH_SAFE(current, swap, _H_BUCKET(table, i), link)



#define H_FOREACH(i, current, swap, table, link)                    \
    for (i = 0; i < _H_CAP(table); i++)                             \
        Q_FOREACH(current, swap, _H_BUCKET(table, i), link)


/****************************************************************
 ***************** PRIVATE HELPER MACROS ************************
 ****************************************************************/

/** @brief Access table field safely */
#define _H_TABLE(table) \
    ((table)->htable)

/** @brief Access size field safely */
#define _H_SIZE(table) \
    ((table)->current_size)

/** @brief Access capacity field safely */
#define _H_CAP(table) \
    ((table)->current_capacity)

/** @brief Accesst key field safely */
#define _H_KEY(elem, key_field) \
    ((elem)->key_field)

/** @brief Get the hash of a key for the current table capacity */
#define _H_HASH(table, key) \
    (hash_int(key) % (table)->current_capacity)

/** @brief Get the bucket assocated with a bucket index */
#define _H_BUCKET(table, hash_val) \
    (_H_TABLE(table) + (hash_val))

/** @def _H_BUCKET_GET(bucket, key, key_field, link_name)
 *
 *  @brief Get an element from a bucket by key
 *
 *  @param bucket The bucket
 *  @param key The key of the element
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @return The element or null
 **/
#define _H_BUCKET_GET(bucket, key, key_field, link_name) \
    ({                                                   \
        typeof(Q_GET_FRONT(bucket)) _search;             \
        int _found = 0;                                  \
        Q_FOREACH(_search, bucket, link_name)            \
        {                                                \
            if (_search->key_field == (key)) {           \
                _found = 1;                              \
                break;                                   \
            }                                            \
        }                                                \
        _found ? _search : NULL;                         \
    })

/** @def _H_BUCKET_INSERT(bucket, elem, lin_name)
 *
 *  @brief Insert an element into a bucket
 *
 *  @param bucket The bucket to add to
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @param elem The element to insert
 *  @return void
 **/
#define _H_BUCKET_INSERT(bucket, elem, link_name) \
    Q_INSERT_FRONT(bucket, elem, link_name)

/** @def _H_BUCKET_REMOVE(bucket, key, key_field, link_name)
 *
 *  @brief Remove an element from a bucket
 *
 *  @param bucket The bucket to remove from
 *  @param key The key of the element to remove
 *  @param key_field The field where the key is stored in the element
 *  @param link_name The name of the link used to organize the bucket lists
 *  @return The removed element
 **/
#define _H_BUCKET_REMOVE(bucket, key, key_field, link_name)     \
    ({                                                          \
        typeof(*Q_GET_FRONT(bucket))* _search;                  \
        int _found = 0;                                         \
        Q_FOREACH(_search, bucket, link_name)                   \
        {                                                       \
            if (_search->key_field == (key)) {                  \
                _found = 1;                                     \
                Q_REMOVE(bucket, _search, link_name);           \
                break;                                          \
            }                                                   \
        }                                                       \
        _found ? _search : NULL;                                \
    })


/** @def _H_ALLOC_TABLE(table, cap)
 *
 *  @brief Allocate space for a hash table
 *
 *  @param table The table to allocate space for
 *  @param cap The amount of space to allocate
 *  @return The allocated space
 **/
#define _H_ALLOC_TABLE(table, cap)                           \
    ({                                                       \
        int _i;                                              \
        typeof(*_H_TABLE(table))* _tmp;                      \
        _tmp = malloc((cap) * sizeof(*_H_TABLE(table)));     \
        if (_tmp != NULL) {                                  \
            for (_i = 0; _i < (cap); _i++) {                 \
                Q_INIT_HEAD(_tmp + _i);                      \
            }                                                \
        }                                                    \
        _tmp;                                                \
    })

/** @def _H_RESIZE_TABLE(table, key_field, link, new_size)
 *
 *  @brief Attempt to change the tables size to new size
 *
 *  On failure to allocate memory, this function simply keeps the table the
 *  same size instead of failing.
 *
 *  @param table The table to resize
 *  @param key_field The field where the key is stored in the element
 *  @param link The name of the link used to organize the bucket lists
 *  @param new_size The new size for the table
 *  @return void
 **/
#define _H_RESIZE_TABLE(table, key_field, link, new_size)                     \
    do {                                                                      \
        int _new_cap = new_size;                                              \
        int _hash, _i;                                                        \
        typeof(*_H_TABLE(table))* _bucket;                                    \
        typeof(*Q_GET_FRONT(_bucket)) *_search;                               \
        typeof(*_H_TABLE(table)) *_tmp = _H_ALLOC_TABLE(table, _new_cap);     \
        if (_tmp != NULL) {                                                   \
            for (_i = 0; _i < _H_CAP(table); _i++) {                          \
                _bucket = _H_BUCKET(table, _i);                               \
                while((_search = Q_GET_FRONT(_bucket)) != NULL){              \
                    Q_REMOVE(_bucket, _search, link);                         \
                    _hash = hash_int(_H_KEY(_search, key_field)) % _new_cap;  \
                    _H_BUCKET_INSERT((_tmp + _hash), _search, link);          \
                }                                                             \
            }                                                                 \
            H_FREE_TABLE(table);                                              \
            _H_TABLE(table) = _tmp;                                           \
            _H_CAP(table) = _new_cap;                                         \
        }                                                                     \
    } while (0)

/** @def _H_GROW(table, key_field, link)
 *
 *  @brief Grow the give table if neccessary
 *  @param key_field The field where the key is stored in the element
 *  @param link The name of the link used to organize the bucket lists
 *  @return void
 **/
#define _H_GROW(table, key_field, link)                                 \
    do {                                                                \
        if (_H_SIZE(table) > MAX_LOAD_FACTOR * _H_CAP(table)) {         \
            _H_RESIZE_TABLE(table, key_field, link, 2 * _H_CAP(table)); \
        }                                                               \
    } while (0)


/** @def _H_SHRINK(table, key_field, link)
 *
 *  @brief Shrink the give table if neccessary
 *  @param key_field The field where the key is stored in the element
 *  @param link The name of the link used to organize the bucket lists
 *  @return void
 **/
#define _H_SHRINK(table, key_field, link)                               \
    do {                                                                \
        if (_H_CAP(table) > MIN_LOAD_FACTOR * _H_SIZE(table)            \
            && _H_CAP(table) > 2 * INITIAL_CAPACITY) {                  \
            _H_RESIZE_TABLE(table, key_field, link, _H_CAP(table) / 2); \
        }                                                               \
    } while (0)
