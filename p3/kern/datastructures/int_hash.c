/** @file int_hash.c
 *
 *  @brief An integer hash function used by the hash table
 *
 *  @author Evan Palmer (esp)
 **/

#include <stdint.h>

/** @brief An integer hash function
 *
 * Hashes integers to integers with some good distrubition properties.
 * Algorithm found at
 *       http://stackoverflow.com/a/12996028
 *
 * @param x The integer to hash
 * @return the hash of the integer
 **/
uint32_t hash_int(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
