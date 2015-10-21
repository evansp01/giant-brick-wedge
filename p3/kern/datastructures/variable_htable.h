#include <stdint.h>
#include "variable_queue.h"
#include <stdlib.h>

#define MAX_LOAD_FACTOR 6 // Seems legit
#define MIN_LOAD_FACTOR 4 // must be 1/4 full
#define INITIAL_CAPACITY 8

uint32_t hash(uint32_t x);

#define H_NEW_TABLE(H_TABLE_TYPE, Q_HEAD_TYPE) \
    typedef struct {                           \
        int current_capacity;                  \
        int current_size;                      \
        Q_HEAD_TYPE* htable;                   \
    } H_TABLE_TYPE

#define H_INIT_TABLE(table)                                             \
    ({                                                                  \
        _H_SIZE(table) = 0;                                             \
        _H_CAP(table) = INITIAL_CAPACITY;                               \
        _H_TABLE(table) = _H_ALLOC_TABLE(table, INITIAL_CAPACITY);      \
        _H_TABLE(table) == NULL;                                        \
    })

#define H_FREE_TABLE(table) \
        free(_H_TABLE(table))

#define H_EMPTY(table) \
    (_H_SIZE(table) == 0)

#define H_SIZE(table) \
    ({ _H_SIZE(table); })

#define H_CAPACITY(table) \
    ({ _H_CAP(table); })


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

#define H_CONTAINS(table, key, key_field, link_name)                \
    ({                                                              \
        typeof(*_H_TABLE(table))* _bucket;                          \
        _bucket = _H_BUCKET(table, _H_HASH(table, key));            \
        _H_BUCKET_GET(_bucket, key, key_field, link_name) != NULL;  \
    })

#define H_GET(table, key, key_field, link_name)                   \
    ({                                                            \
        typeof(*_H_TABLE(table))* _bucket;                        \
        _bucket = _H_BUCKET(table, _H_HASH(table, key));          \
        _H_BUCKET_GET(_bucket, key, key_field, link_name);        \
    })


// PRIVATE HELPER MACROS

#define _H_TABLE(table) \
    ((table)->htable)

#define _H_SIZE(table) \
    ((table)->current_size)

#define _H_CAP(table) \
    ((table)->current_capacity)

#define _H_KEY(elem, key_field) \
    ((elem)->key_field)

#define _H_HASH(table, key) \
    (hash(key) % (table)->current_capacity)

#define _H_GET_BUCKET(table, elem, key_field) \
    _H_BUCKET(table, _H_HASH(table, _H_KEY(elem, key_field)))

#define _H_BUCKET(table, hash_val) \
    (_H_TABLE(table) + hash_val)

#define _H_BUCKET_GET(bucket, key, key_field, link_name) \
    ({                                                   \
        typeof(Q_GET_FRONT(bucket)) _search;             \
        int _found = 0;                                  \
        Q_FOREACH(_search, bucket, link_name)            \
        {                                                \
            if (_search->key_field == key) {             \
                _found = 1;                              \
                break;                                   \
            }                                            \
        }                                                \
        _found ? _search : NULL;                         \
    })

#define _H_BUCKET_INSERT(bucket, elem, link_name) \
    Q_INSERT_FRONT(bucket, elem, link_name)

#define _H_BUCKET_REMOVE(bucket, key, key_field, link_name)     \
    ({                                                          \
        typeof(*Q_GET_FRONT(bucket))* _search;                  \
        int _found = 0;                                         \
        Q_FOREACH(_search, bucket, link_name)                   \
        {                                                       \
            if (_search->key_field == key) {                    \
                _found = 1;                                     \
                Q_REMOVE(bucket, _search, link_name);           \
                break;                                          \
            }                                                   \
        }                                                       \
        _found ? _search : NULL;                                \
    })



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

#define _H_RESIZE_TABLE(table, key_field, link, new_size)                   \
    do {                                                                    \
        int _new_cap = new_size;                                            \
        int _hash, _i;                                                      \
        typeof(*_H_TABLE(table))* _bucket;                                  \
        typeof(*Q_GET_FRONT(_bucket)) *_search;                             \
        typeof(*_H_TABLE(table)) *_tmp = _H_ALLOC_TABLE(table, _new_cap);   \
        if (_tmp != NULL) {                                                 \
            for (_i = 0; _i < _H_CAP(table); _i++) {                        \
                _bucket = _H_BUCKET(table, _i);                             \
                while((_search = Q_GET_FRONT(_bucket)) != NULL){            \
                    Q_REMOVE(_bucket, _search, link);                       \
                    _hash = hash(_H_KEY(_search, key_field)) % _new_cap;    \
                    _H_BUCKET_INSERT((_tmp + _hash), _search, link);        \
                }                                                           \
            }                                                               \
            free(_H_TABLE(table));                                          \
            _H_TABLE(table) = _tmp;                                         \
            _H_CAP(table) = _new_cap;                                       \
        }                                                                   \
    } while (0)

#define _H_GROW(table, key_field, link)                                 \
    do {                                                                \
        if (_H_SIZE(table) > MAX_LOAD_FACTOR * _H_CAP(table)) {         \
            _H_RESIZE_TABLE(table, key_field, link, 2 * _H_CAP(table)); \
        }                                                               \
    } while (0)


#define _H_SHRINK(table, key_field, link)                               \
    do {                                                                \
        if (_H_CAP(table) > MIN_LOAD_FACTOR * _H_SIZE(table)            \
            && _H_CAP(table) > 2 * INITIAL_CAPACITY) {                  \
            _H_RESIZE_TABLE(table, key_field, link, _H_CAP(table) / 2); \
        }                                                               \
    } while (0)

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
