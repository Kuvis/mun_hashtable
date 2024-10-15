/* =============================================================================
 * This file is part of Mun Hashtable.
 *
 * Mun Hashtable is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Mun Hashtable is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Mun Hashtable.  If not, see <https://www.gnu.org/licenses/>.
 * ===========================================================================*/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "hashtable.h"

#define HASHTABLE_LOAD_FACTOR   70
#define HASHTABLE_GROWTH_FACTOR 2

static void _hashtable_default_panic(void);

void (*hashtable_panic)(void) = _hashtable_default_panic;

size_t hashtable_hash(const void *key, size_t size)
{
#if UINTPTR_MAX == 0xFFFFFFFF
    size_t hash = 0x811C9DC5;
    for (size_t i = 0; i < size; ++i) {
        hash ^= *((unsigned char*)key + i);
        hash *= 0x01000193;
    }
    return hash;
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
    size_t hash = 0xcbf29ce484222325;
    for (size_t i = 0; i < size; ++i) {
        hash ^= *((unsigned char*)key + i);
        hash *= 0x100000001b3;
    }
    return hash;
#else
  #error Only 32-bit and 64-bit builds are supported.
#endif
}

size_t hashtable_str_hash(const char *key)
{
#if UINTPTR_MAX == 0xFFFFFFFF
    size_t hash = 0x811C9DC5;
    for (const char *c = key; *c; ++c) {
        hash ^= *c;
        hash *= 0x01000193;
    }
    return hash;
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
    size_t hash = 0xcbf29ce484222325;
    for (const char *c = key; *c; ++c) {
        hash ^= *c;
        hash *= 0x100000001b3;
    }
    return hash;
#else
  #error Only 32-bit and 64-bit builds are supported.
#endif
}

int hashtable_copy_key(void *dst, const void *src, size_t size)
{
    memcpy(dst, src, size);
    return 0;
}

int hashtable_compare_keys(const void *a, const void *b, size_t size)
    {return memcmp(a, b, size);}

void *_hashtable_init(size_t *num_buckets, size_t num, size_t bucket_size,
    size_t *num_values, int *ret_err)
{
    void *ret = calloc(num, bucket_size);
    if (!ret && num) {
        if (ret_err)
            *ret_err = 1;
        return ret;
    }
    *num_buckets = num;
    *num_values = 0;
    if (ret_err)
        *ret_err = 0;
    return ret;
}

void _hashtable_clear(unsigned char *buckets, size_t num_buckets,
    size_t bucket_size, size_t *num_values, size_t key_off, size_t hash_off,
    void (*free_key)(void *key))
{
    if (!free_key) {
        for (size_t i = 0; i < num_buckets; ++i) {
            unsigned char *bucket = buckets + i * bucket_size;
            memset(bucket + hash_off, 0, sizeof(size_t));
        }
    } else {
        const size_t zero = 0;
        for (size_t i = 0; i < num_buckets; ++i) {
            unsigned char *bucket = buckets + i * bucket_size;
            if (memcmp(bucket + hash_off, &zero, sizeof(size_t))) {
                free_key(bucket + key_off);
                memset(bucket + hash_off, 0, sizeof(size_t));
            }
        }
    }
    *num_values = 0;
}

void _hashtable_destroy(void *table, size_t table_size, unsigned char *buckets,
    void (*free_key)(void *), size_t num_buckets, size_t bucket_size,
    size_t key_off, size_t hash_off, size_t num_values)
{
    if (free_key && num_values) {
        const size_t zero = 0;
        for (size_t i = 0; i < num_buckets; ++i) {
            unsigned char *bucket = buckets + i * bucket_size;
            if (memcmp(bucket + hash_off, &zero, sizeof(size_t)))
                free_key(bucket + key_off);
        }
    }
    free(buckets);
    memset(table, 0, table_size);
}

void *_hashtable_insert(int *ret_err, unsigned char *HASHTABLE_RESTRICT buckets,
    size_t *HASHTABLE_RESTRICT num_buckets,
    size_t *HASHTABLE_RESTRICT num_values, size_t bucket_size, size_t key_off,
    size_t value_off, size_t hash_off, void *HASHTABLE_RESTRICT key,
    size_t key_size, size_t hash, void *HASHTABLE_RESTRICT value,
    size_t value_size,
    int (*compare_keys)(const void *a, const void *b, size_t size),
    int (*copy_key)(void *dst, const void *src, size_t size))
{
    if (!hash) {
        if (ret_err)
            *ret_err = 1;
        return buckets;
    }
    /* Resize if num_values / num_buckets >= HASHTABLE_LOAD_FACTOR percent */
    if (!*num_buckets ||
        (size_t)100 * (*num_values) / (*num_buckets) >= HASHTABLE_LOAD_FACTOR)
    {
        size_t num_new_buckets = (*num_buckets) *
            (HASHTABLE_GROWTH_FACTOR * 100) / 100;
        if (num_new_buckets == 0)
            num_new_buckets = 8;
        else if (num_new_buckets == *num_buckets)
            num_new_buckets = 2 * (*num_buckets);
        unsigned char *new_buckets = calloc(num_new_buckets, bucket_size);
        if (!new_buckets) {
            if (ret_err)
                *ret_err = 4;
            return buckets;
        }
        for (size_t i = 0; i < (*num_buckets); ++i) {
            unsigned char *old_bucket = buckets + i * bucket_size;
            size_t  old_hash;
            memcpy(&old_hash, old_bucket + hash_off, sizeof(hash));
            if (!old_hash) /* Bucket not in use */
                continue;
            for (size_t j = old_hash % num_new_buckets;;) {
                unsigned char *new_bucket = new_buckets + j * bucket_size;
                size_t new_item_hash;
                memcpy(&new_item_hash, new_bucket + hash_off,
                    sizeof(new_item_hash));
                if (!new_item_hash) {
                    memcpy(new_bucket, old_bucket, bucket_size);
                    break;
                }
                j = (j + 1) % num_new_buckets;
                assert(j != old_hash % num_new_buckets);
            }
        }
        free(buckets);
        buckets         = new_buckets;
        *num_buckets    = num_new_buckets;
    }
    /* Find a free slot now that we're sure there's space */
    size_t bucket_index = (size_t)(hash % (size_t)(*num_buckets));
    for (size_t i = bucket_index;;) {
        unsigned char *bucket = buckets + i * bucket_size;
        size_t item_hash;
        memcpy(&item_hash, bucket + hash_off, sizeof(item_hash));
        if (!item_hash) {
            if (copy_key(bucket + key_off, key, key_size)) {
                if (ret_err)
                    *ret_err = 3;
                return buckets;
            }
            memcpy(bucket + value_off, value, value_size);
            memcpy(bucket + hash_off, &hash, sizeof(size_t));
            if (ret_err)
                *ret_err = 0;
            (*num_values)++;
            return buckets;
        } else if (!compare_keys(bucket + key_off, key, key_size)) {
            /* Key already exists */
            if (ret_err)
                *ret_err = 2;
            return buckets;
        }
        i = (i + 1) % *num_buckets;
        assert(i != (bucket_index));
    }
}

void *_hashtable_find(const void *HASHTABLE_RESTRICT key, size_t key_size,
    size_t hash, unsigned char *HASHTABLE_RESTRICT buckets, size_t num_buckets,
    size_t bucket_size, size_t key_off, size_t value_off, size_t hash_off,
    int (*compare_keys)(const void *a, const void *b, size_t size))
{
    if (!num_buckets)
        return 0;
    size_t bucket_index = hash % num_buckets;
    for (size_t i = bucket_index;;) {
        unsigned char *bucket = buckets + i * bucket_size;
        size_t item_hash;
        memcpy(&item_hash, bucket + hash_off, sizeof(item_hash));
        if (!item_hash)
            return 0;
        if (!compare_keys(bucket + key_off, key, key_size))
            return bucket + value_off;
        i = (i + 1) % num_buckets;
        if (i == bucket_index)
            return 0;
    }
    return 0;
}

void _hashtable_erase(unsigned char *HASHTABLE_RESTRICT buckets,
    size_t num_buckets, size_t *HASHTABLE_RESTRICT num_values,
    void *HASHTABLE_RESTRICT key, size_t key_size, size_t hash,
    size_t bucket_size, size_t key_off, size_t hash_off,
    int (*compare_keys)(const void *a, const void *b, size_t size),
    void (*free_key)(void *key))
{
    if (!*num_values)
        return;
    size_t bucket_index = hash % num_buckets;
    for (size_t i = bucket_index; ; i = (i + 1) % num_buckets) {
        unsigned char *bucket = buckets + i * bucket_size;
        size_t item_hash;
        memcpy(&item_hash, bucket + hash_off, sizeof(item_hash));
        if (compare_keys(bucket + key_off, key, key_size))
            continue;
        if (free_key)
            free_key(bucket + key_off);
        memset(bucket, 0, bucket_size);
        /* Move any neighbouring buckets back if they have been pushed forward
         * previously. */
        unsigned char *previous_bucket = bucket;
        for (size_t j = (i + 1) % num_buckets;;) {
            unsigned char *other_bucket = buckets + j * bucket_size;
            size_t other_hash;
            memcpy(&other_hash, other_bucket + hash_off, sizeof(other_hash));
            if (!other_hash)
                break;
            size_t other_bucket_index = other_hash % num_buckets;
            /* If the neighboring bucket's index is not the current index, it
             * must have been moved forward, so we now move it to the previous
             * free slot. */
            if (other_bucket_index != j) {
                memcpy(previous_bucket, other_bucket, bucket_size);
                memset(other_bucket + hash_off, 0, sizeof(size_t));
                previous_bucket = other_bucket;
            }
            j = (j + 1) % num_buckets;
        }
        (*num_values)--;
        return;
    }
}

int _hashtable_for_each_pair(size_t *HASHTABLE_RESTRICT i,
    size_t *HASHTABLE_RESTRICT j, void *HASHTABLE_RESTRICT ret_key,
    void *HASHTABLE_RESTRICT ret_value, size_t key_size, size_t value_size,
    size_t num_values, unsigned char *HASHTABLE_RESTRICT buckets,
    size_t bucket_size, size_t key_off, size_t hash_off, size_t value_off)
{
    /* i = bucket
     * j = value_num */
    if (*j >= num_values)
        return 0;
    for (;;) {
        unsigned char *bucket = buckets + (*i) * bucket_size;
        size_t item_hash;
        memcpy(&item_hash, bucket + hash_off, sizeof(item_hash));
        ++(*i);
        if (!item_hash)
            continue;
        memcpy(ret_key, bucket + key_off, key_size);
        memcpy(ret_value, bucket + value_off, value_size);
        ++(*j);
        return 1;
    }
}

static void _hashtable_default_panic(void)
    {abort();}
