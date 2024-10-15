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

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>

/* =============================================================================
 * hashtable()
 * Declare a new hashtable variable.
 *
 * PARAMETERS
 * key_type:    The type of the key used by this table.
 * value_type:  The type of the values contained by this table.
 *
 * EXAMPLE
 * hashtable(uint32_t, void*) my_table;
 * ===========================================================================*/
#define hashtable(key_type, value_type) \
    struct { \
        _hashtable_body(key_type, value_type) \
    }

/* =============================================================================
 * hashtable_init()
 * Initialize a hashtable.
 *
 * PARAMETERS
 * table:   The hashtable to initialize.
 * size:    Number of initial buckets.
 * ret_err: A pointer to an int to which a potential error code is written. Can
 *          be NULL. A value of 0 indicates success.
 *
 * RETURN VALUE
 * void
 * ===========================================================================*/
#define hashtable_init(table, size, ret_err) \
    ((void)((table)._buckets = _hashtable_init(&(table)._num_buckets, (size), \
        sizeof(*(table)._buckets), &(table)._num_values, (ret_err))))

/* =============================================================================
 * hashtable_einit()
 * Same as hashtable_init(), but does not return an error status, instead
 * calling the function pointed to by hashtable_panic in case something goes
 * wrong.
 * ===========================================================================*/
#define hashtable_einit(table, size) \
    ((void)((table)._buckets = _hashtable_einit(&(table)._num_buckets, (size), \
        sizeof((table)._buckets[0]), &(table)._num_values)))

/* =============================================================================
 * hashtable_clear()
 * Delete all entries in a table without freeing memory.
 *
 * PARAMETERS
 * table:       The hashtable to clear.
 * free_key:    A pointer to a function to free keys of entries in case the keys
 *              were dynamically allocated using a custom key copying function.
 *              Can be NULL. Must have the following signature:
 *              void free_key(void *key);
 *
 * RETURN VALUE
 * void
 * ===========================================================================*/
#define hashtable_clear(table, free_key) \
    _hashtable_clear((unsigned char*)(table)._buckets, (table)._num_buckets, \
        sizeof((table)._buckets[0]), &(table)._num_values, \
        _hashtable_ptr_offset(&(table)._buckets[0]._key, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._hash, \
            &(table)._buckets[0]), \
        free_key)

#define hashtable_num_buckets(table) \
    (table._num_buckets)

#define hashtable_num_values(table) \
    (table._num_values)

/* =============================================================================
 * hashtable_destroy()
 * Free resources used by a hashtable.
 *
 * PARAMETERS
 * table:       The hashtable to destroy.
 * free_key:    A pointer to a function to free keys of entries in case the keys
 *              were dynamically allocated using a custom key copying function.
 *              Can be NULL. Must have the following signature:
 *              void free_key(void *key);
 *
 * RETURN VALUE
 * void
 * ===========================================================================*/
#define hashtable_destroy(table, free_key) \
    _hashtable_destroy(&table, sizeof(table), \
        (unsigned char*)(table)._buckets, free_key, (table)._num_buckets, \
        sizeof((table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._key, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._hash, \
            &(table)._buckets[0]), (table)._num_values)

/* =============================================================================
 * hashtable_insert()
 * Insert a key-hash-value combination into the table. Any hash function that
 * returns a size_t may be used to generate the hash.
 * This macro is for fixed-size key types - for variable length types such as
 * strings, see hashtable_insert_ext().
 *
 * PARAMETERS
 * table:   The hashtable
 * key:     The key used to compute the hash. Must refer to an existing
 *          variable of the correct type.
 * hash:    The hash computed from the key. Must be of type size_t.
 * value:   The value to be inserted. Must refer to an existing variable of the
 *          correct type.
 * ret_err: A pointer to an int to write a return code to. NULL if none. A value
 *          of 0 indicates success.
 *
 * RETURN VALUE
 * void
 *
 * EXAMPLE
 * size_t hash_u32(uint32_t key);           // Custom hash function
 * hashtable(uint32_t, void *) my_table;    // The table to insert into
 * ...
 * uint32_t  key     = 324;
 * size_t    hash    = hash_u32(key);
 * void      *value  = NULL;
 * hashtable_insert(my_table, key, hash, value, NULL);
 * ===========================================================================*/
#define hashtable_insert(table, key, hash, value, ret_err) \
    hashtable_insert_ext(table, key, hash, value, \
        hashtable_compare_keys, hashtable_copy_key, ret_err)


/* =============================================================================
 * hashtable_einsert()
 * Similar to hashtable_insert(), but will never report failure, instead calling
 * the function pointer to by hashtable_panic if something goes wrong.
 * ===========================================================================*/
#define hashtable_einsert(table, key, hash, value) \
    hashtable_einsert_ext(table, key, hash, value, hashtable_compare_keys, \
        hashtable_copy_key)

/* =============================================================================
 * hashtable_insert_ext()
 * Similar to hashtable_insert(), but uses custom key comparison and key
 * duplication functions.
 *
 * PARAMETERS
 * table:           The hashtable
 * key:             The key used to compute the hash. Must refer to an existing
 *                  variable of the correct type.
 * hash:            The hash computed from the key. Must be of type size_t.
 * value:           The value to be inserted. Must refer to an existing variable
 *                  of the correct type.
 * compare_keys:    A pointer to a function that checks if keys are equal. A
 *                  return value of 0 indicates the keys are equal. The function
 *                  signature must be as follows:
 *                  int compare_keys(const void *a, const void *b, size_t size);
 *                  hashtable_insert() uses the function
 *                  hashtable_compare_keys().
 * copy_key:        A pointer to a function that copies the key passed to the
 *                  macro. The return value must be 0 on success. The signature
 *                  must be as follows:
 *                  int copy_key(void *dst, const void *src, size_t size);
 *                  hashtable_insert() uses the function hashtable_copy_keys().
 * ret_err:         A pointer to an int to write a return code to. NULL if none.
 *                  A value of 0 indicates success.
 *
 * RETURN VALUE
 * void
 * ===========================================================================*/
#define hashtable_insert_ext(table, key, hash, value, \
    compare_keys, copy_key, ret_err) \
    ((void)((table)._buckets = _hashtable_insert((ret_err), \
        (unsigned char*)(table)._buckets, \
        &(table)._num_buckets, &(table)._num_values, \
        sizeof(*(table)._buckets), \
        _hashtable_ptr_offset(&(table)._buckets[0]._key, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._value, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._hash, \
            &(table)._buckets[0]), \
        &key, sizeof(key), hash, &value, sizeof(value), compare_keys, \
        copy_key)))

#define hashtable_einsert_ext(table, key, hash, value, compare_keys, copy_key) \
    ((void)((table)._buckets = _hashtable_einsert( \
        (unsigned char*)(table)._buckets, &(table)._num_buckets, \
        &(table)._num_values, sizeof(*(table)._buckets), \
        _hashtable_ptr_offset(&(table)._buckets[0]._key, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._value, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._hash, \
            &(table)._buckets[0]), \
        &key, sizeof(key), hash, &value, sizeof(value), compare_keys, \
        copy_key)))

/* =============================================================================
 * hashtable_find()
 * Find a value by key in the given hashtable.
 *
 * PARAMETERS
 * table:   The hashtable to find from
 * key:     The key used to compute the hash. Must refer to an existing variable
 *          of the correct type.
 * hash:    The hash computed from the key. Must be of type size_t.
 *
 * RETURN VALUE
 * A void * to the value, or NULL if value is not found.
 *
 * EXAMPLE
 * size_t hash_u32(uint32_t key);           // Custom hash function
 * hashtable(uint32_t, void *) my_table;    // The table to find values from
 * ...
 * uint32_t key = 324;
 * void **value = hashtable_find(my_table, key, hash_u32(key));
 * if (value) {
 *     ... Value found ...
 * }
 * ===========================================================================*/
#define hashtable_find(table, key, hash) \
    hashtable_find_ext(table, key, hash, hashtable_compare_keys)

/* =============================================================================
 * hashtable_find_ext()
 * Like hashtable_find(), but uses a custom key comparison function.
 *
 * PARAMETERS
 * table:           The hashtable to find from
 * key:             The key used to compute the hash. Must refer to an existing
 *                  variable of the correct type.
 * hash:            The hash computed from the key. Must be of type size_t.
 * compare_keys:    A pointer to a function to compare two keys with each other.
 *                  Must return 0 if keys are equal. Signature must be as
 *                  follows:
 *                  int compare_keys(const void *a, const void *b, size_t size);
 *
 * RETURN VALUE
 * A void * to the value, or NULL if value is not found.
 * ===========================================================================*/
#define hashtable_find_ext(table, key, hash, compare_keys) \
    _hashtable_find(&key, sizeof(key), \
        hash, (unsigned char*)(table)._buckets, (table)._num_buckets, \
        sizeof((table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._key, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._value, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._hash, \
            &(table)._buckets[0]), \
            compare_keys)

/* =============================================================================
 * hashtable_erase()
 * Erase a key-value pair from a hashtable.
 *
 * PARAMETERS
 * table:   The hashtable to erase from.
 * key:     The key used to compute the hash. Must refer to an existing variable
 *          of the correct type.
 * hash:    The hash computed from the key. Must be of type size_t.
 *
 * RETURN VALUE
 * void
 *
 * EXAMPLE
 * size_t hash_u32(uint32_t key);       // Custom hash function
 * hashtable(uint32_t, int) my_table;   // The table to erase from
 * ...
 * uint32_t key = 324;
 * hashtable_erase(my_table, key, hash_u32(key));
 * ===========================================================================*/
#define hashtable_erase(table, key, hash) \
    hashtable_erase_ext(table, key, hash, hashtable_compare_keys, 0)

/* =============================================================================
 * hashtable_erase_ext()
 * Like hashtable_erase(), but uses custom key comparison and key freeing
 * functions.
 *
 * PARAMETERS
 * table:           The hashtable to erase from.
 * key:             The key used to compute the hash. Must refer to an existing
 *                  variable of the correct type.
 * hash:            The hash computed from the key. Must be of type size_t.
 * compare_keys:    A pointer to a function to check if two keys are equal. Must
 *                  return 0 if keys are equal. Must have the following
 *                  signature:
 *                  int compare_keys(const void *a, const void *b, size_t size);
 * free_key:        A pointer to a function to free a key when a key-value
 *                  combination is erased. This can be used if a custom key was
 *                  dynamically allocated using a custom copy_key function
 *                  during insertion. If this parameter is NULL, no function is
 *                  called to free the key. The function signature must be as
 *                  follows:
 *                  void free_key(void *key);
 *
 * RETURN VALUE
 * void
 * ===========================================================================*/
#define hashtable_erase_ext(table, key, hash, compare_keys, free_key) \
    _hashtable_erase((unsigned char*)(table)._buckets, (table)._num_buckets, \
        &(table)._num_values, &key, sizeof(key), \
        hash, \
        sizeof((table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._key, \
            &(table)._buckets[0]), \
        _hashtable_ptr_offset(&(table)._buckets[0]._hash, \
            &(table)._buckets[0]), \
        compare_keys, free_key)

/* =============================================================================
 * hashtable_exists()
 * Check if a key-value pair has been inserted into the table.
 *
 * PARAMETERS
 * table:   The hashtable to find from
 * key:     The key used to compute the hash. Must refer to an existing variable
 *          of the correct type.
 * hash:    The hash computed from the key. Must be of type size_t.
 *
 * RETURN VALUE
 * 1 if pair exists, 0 if not.
 * ===========================================================================*/
#define hashtable_exists(table, key, hash) \
    (hashtable_find(table, key, hash) ? 1 : 0)

/* =============================================================================
 * hashtable_exists_ext()
 * Like hashtable_exists(), but uses a custom key comparison function.
 *
 * PARAMETERS
 * table:           The hashtable to find from
 * key:             The key used to compute the hash. Must refer to an
 *                  existing variable of the correct type.
 * hash:            The hash computed from the key. Must be of type size_t.
 * compare_keys:    A pointer to a function to check if two keys are equal. Must
 *                  return 0 if keys are equal. Must have the following
 *                  signature:
 *                  int compare_keys(const void *a, const void *b, size_t size);
 *
 * RETURN VALUE
 * 1 if pair exists, 0 if not.
 * ===========================================================================*/
#define hashtable_exists_ext(table, key, hash, compare_keys) \
    (hashtable_find_ext(table, key, hash, compare_keys) ? 1 : 0)

/* =============================================================================
 * hashtable_for_each_pair()
 * Iterate through each key-value pair in the table. The table must not be
 * modified using insert/erase during iteration.
 *
 * EXAMPLE
 * hashtable(uint32_t, int) my_table;
 * ...
 * uint32  key;
 * int     value;
 * hashtable_for_each_pair(my_table, key, value) {
 *     ... Do something with key and value ...
 * }
 * ===========================================================================*/
#define hashtable_for_each_pair(table, ret_key, ret_value) \
    for (size_t hashtable_i__ = 0, hashtable_j__ = 0; \
        _hashtable_for_each_pair(&hashtable_i__, &hashtable_j__, &ret_key, \
            &ret_value, sizeof(table._buckets[0]._key), \
            sizeof(table._buckets[0]._value), table._num_values, \
            (unsigned char*)table._buckets, sizeof(table._buckets[0]), \
            _hashtable_ptr_offset(&table._buckets[0]._key, \
                &table._buckets[0]), \
            _hashtable_ptr_offset(&table._buckets[0]._hash, \
                &table._buckets[0]), \
            _hashtable_ptr_offset(&table._buckets[0]._value, \
                &table._buckets[0]));)

/* =============================================================================
 * hashtable_define()
 * A macro for defining typesafe hashtables and functions for their use. This
 * macro defines a hashtable type of the given name, and the following functions
 * where
 * - TABLE represents the type name of the table, which is the first macro
 *   parameter
 * - VALUE_TYPE represents the type contained in the table
 * - KEY_TYPE represents the type used as key by the table
 *
 * int TABLE_init(TABLE *table, size_t size)
 * Same as hashtable_init(), but directly returns an error code (zero means
 * success).
 *
 * void TABLE_einit(TABLE *table, size_t size)
 * Same as hashtable_einit().
 *
 * void TABLE_destroy(TABLE *table)
 * Same as hashtable_destroy().
 *
 * int TABLE_insert(TABLE *table, KEY_TYPE key, VALUE_TYPE value)
 * Same as hashtable_insert_ext(), but directly returns an error code (zero
 * means success).
 *
 * void TABLE_einsert(TABLE *table, KEY_TYPE key, VALUE_TYPE value)
 * Same as hashtable_einsert_ext().
 *
 * void TABLE_erase(TABLE *table, KEY_TYPE key)
 * Same as hashtable_erase_ext().
 *
 * int TABLE_exists(TABLE *table, KEY_TYPE key)
 * Sameas hashtable_exists().
 *
 * VALUE_TYPE *TABLE_find(TABLE *table, KEY_TYPE key)
 * Same as hashtable_find_ext().
 *
 * PARAMETERS
 * table_type_name: The type name and function prefix used for the table.
 * key_type:        The type used as key for the table.
 * value_type:      The type of the values that will be  stored in the table.
 *
 * EXAMPLE
 * hashtable_define(int_int_table, int, int);
 * ...
 * struct int_int_table my_table;
 * int_int_table_einit(&my_table, 8);
 * ===========================================================================*/
#define hashtable_define(table_type_name, key_type, value_type) \
    hashtable_define_ext(table_type_name, key_type, value_type, \
        hashtable_hash, hashtable_compare_keys, hashtable_copy_key, 0)

/* =============================================================================
 * hashtable_define_ext()
 * Similar to hashtable_define(), but accepts extra parameters in the form of
 * functions the table will use to compute hashes and compare, copy and free
 * keys.
 *
 * PARAMETERS
 * table_type_name: The type name and function prefix used for the table.
 * key_type:        The type used as key for the table.
 * value_type:      The type of the values that will be  stored in the table.
 * compute_hash:    The function used to compute hashes from keys. The signature
 *                  of this function must be as follows:
 *                  size_t compute_hash(const void *data, size_t size)
 *                  A pointer to the key is passed as parameter data, and the
 *                  size of the key's type is passed as parameter size. Hence,
 *                  if the size of the type itself is not relevant, the size
 *                  parameter can be ignored  (in the case of variable length
 *                  strings, for example).
 * compare_keys:    A pointer to a function that checks if two keys are equal. A
 *                  return value of 0 indicates the keys are equal. The function
 *                  signature must be as follows:
 *                  int compare_keys(const void *a, const void *b, size_t size);
 *                  hashtable_insert() uses the function
 *                  hashtable_compare_keys().
 * copy_key:        A pointer to a function that copies a key when an item is
 *                  inserted into the table. Function signature must be as
 *                  follows:
 *                  int copy_key(void *dst, const void *src, size_t size);
 *                  hashtable_insert() uses the function hashtable_copy_keys().
 * free_key:        A pointer to a function to free a key when a key-value
 *                  combination is erased. This can be used if a custom key was
 *                  dynamically allocated using a custom copy_key function
 *                  during insertion. If this parameter is NULL, no function is
 *                  called to free the key. The function signature must be as
 *                  follows:
 *                  void free_key(void *key);
 * ===========================================================================*/
#define hashtable_define_ext(table_type_name, key_type, value_type, \
    compute_hash, compare_keys, copy_key, free_key) \
    \
    struct table_type_name { \
        _hashtable_body(key_type, value_type) \
    }; \
    \
    static inline int table_type_name##_init(struct table_type_name *table, \
        size_t size) \
    { \
        int err; \
        hashtable_init(*table, size, &err); \
        return err; \
    } \
    \
    static inline void table_type_name##_einit(struct table_type_name *table, \
        size_t size) \
        {hashtable_einit(*table, size);} \
    \
    static inline void table_type_name##_destroy( \
        struct table_type_name *table) \
        {hashtable_destroy(*table, free_key);} \
    \
    static inline int table_type_name##_insert(struct table_type_name *table, \
        key_type key, value_type value) \
    { \
        int err; \
        size_t hash = compute_hash(&key, sizeof(key)); \
        hashtable_insert_ext(*table, key, hash, value, compare_keys, copy_key, \
            &err); \
        return err; \
    } \
    \
    static inline void table_type_name##_einsert( \
        struct table_type_name *table, key_type key, value_type value) \
    { \
        size_t hash = compute_hash(&key, sizeof(key)); \
        hashtable_einsert_ext(*table, key, hash, value, compare_keys, \
            copy_key); \
    } \
    \
    static inline void table_type_name##_erase(struct table_type_name *table, \
        key_type key) \
    { \
        size_t hash = compute_hash(&key, sizeof(key)); \
        hashtable_erase_ext(*table, key, hash, compare_keys, free_key); \
    } \
    \
    static inline int table_type_name##_exists(struct table_type_name *table, \
        key_type key) \
    { \
        size_t hash = compute_hash(&key, sizeof(key)); \
        return hashtable_exists(*table, key, hash); \
    } \
    \
    static inline value_type *table_type_name##_find( \
        struct table_type_name *table, \
        key_type key) \
    { \
        size_t hash = compute_hash(&key, sizeof(key)); \
        return hashtable_find_ext(*table, key, hash, compare_keys); \
    } \
    \
    static inline void table_type_name##_clear(struct table_type_name *table) \
    { \
        hashtable_clear(*table, free_key); \
    }

/* =============================================================================
 * hashtable_hash()
 * A default hash function. Uses the 32 bit or 64 bit fnv-a1 algorithm depending
 * on architecture.
 * ===========================================================================*/
size_t hashtable_hash(const void *key, size_t size);

/* =============================================================================
 * hashtable_str_hash()
 * A hash function for strings. Uses the same algorithm as * hashtable_hash(),
 * but saves having to first calculate the string's length just to pass it to
 * hashtable_hash().
 * ===========================================================================*/
size_t hashtable_str_hash(const char *key);

/* =============================================================================
 * hashtable_compare_keys()
 * The default key comparison function. Compares values like memcmp() does.
 * ===========================================================================*/
int hashtable_compare_keys(const void *a, const void *b, size_t size);

/* =============================================================================
 * hashtable_copy_key()
 * The default key copying function. Works similarly to memcpy().
 * ===========================================================================*/
int hashtable_copy_key(void *dst, const void *src, size_t size);

/* =============================================================================
 * hashtable_panic()
 * The panic function used by any of the errorless functions (hashtable_einsert,
 * etc.) This may be set manually to something else, or left unchanged, in which
 * case it will call standard abort().
 * ===========================================================================*/
extern void (*hashtable_panic)(void);

#define _hashtable_body(key_type, value_type) \
    struct { \
        key_type    _key; \
        value_type  _value; \
        size_t      _hash; \
    } *_buckets; \
    size_t _num_buckets; \
    size_t _num_values;

#define _hashtable_ptr_offset(ptr, base) \
    ((size_t)((unsigned char*)(ptr) - (unsigned char*)(base)))

#ifndef _MSC_VER
  #define HASHTABLE_RESTRICT restrict
#else
  #define HASHTABLE_RESTRICT __restrict
#endif

void *_hashtable_init(size_t *num_buckets, size_t num,
    size_t bucket_size, size_t *num_values, int *ret_err);

static inline void *_hashtable_einit(size_t *HASHTABLE_RESTRICT num_buckets, size_t num,
    size_t bucket_size, size_t *HASHTABLE_RESTRICT num_values);

void _hashtable_clear(unsigned char *buckets, size_t num_buckets,
    size_t bucket_size, size_t *num_values, size_t key_off, size_t hash_off,
    void (*free_key)(void *key));

void _hashtable_destroy(void *table, size_t table_size, unsigned char *buckets,
    void (*free_key)(void *), size_t num_buckets, size_t bucket_size,
    size_t key_off, size_t hash_off, size_t num_values);

void *_hashtable_insert(int *ret_err, unsigned char *HASHTABLE_RESTRICT buckets,
    size_t *HASHTABLE_RESTRICT num_buckets, size_t *HASHTABLE_RESTRICT num_values,
    size_t bucket_size, size_t key_off1, size_t value_off1, size_t hash_off1,
    void *HASHTABLE_RESTRICT key, size_t key_size, size_t hash, void *HASHTABLE_RESTRICT value,
    size_t value_size,
    int (*compare_keys)(const void *a, const void *b, size_t size),
    int (*copy_key)(void *dst, const void *src, size_t size));

static inline void *_hashtable_einsert(unsigned char *HASHTABLE_RESTRICT buckets,
    size_t *HASHTABLE_RESTRICT num_buckets, size_t *HASHTABLE_RESTRICT num_values,
    size_t bucket_size, size_t key_off1, size_t value_off1, size_t hash_off1,
    void *HASHTABLE_RESTRICT key, size_t key_size, size_t hash, void *HASHTABLE_RESTRICT value,
    size_t value_size,
    int (*compare_keys)(const void *a, const void *b, size_t size),
    int (*copy_key)(void *dst, const void *src, size_t size));

void *_hashtable_find(const void *HASHTABLE_RESTRICT key, size_t key_size, size_t hash,
    unsigned char *HASHTABLE_RESTRICT buckets, size_t num_buckets, size_t bucket_size,
    size_t key_off, size_t value_off, size_t hash_off,
    int (*compare_keys)(const void *a, const void *b, size_t size));

void _hashtable_erase(unsigned char *HASHTABLE_RESTRICT buckets, size_t num_buckets,
    size_t *HASHTABLE_RESTRICT num_values, void *HASHTABLE_RESTRICT key, size_t key_size,
    size_t hash, size_t bucket_size, size_t key_off, size_t hash_off,
    int (*compare_keys)(const void *a, const void *b, size_t size),
    void (*free_key)(void *key));

int _hashtable_for_each_pair(size_t *HASHTABLE_RESTRICT i, size_t *HASHTABLE_RESTRICT j,
    void *HASHTABLE_RESTRICT ret_key, void *HASHTABLE_RESTRICT ret_value, size_t key_size,
    size_t value_size, size_t num_values, unsigned char *HASHTABLE_RESTRICT buckets,
    size_t bucket_size, size_t key_off, size_t hash_off, size_t value_off);

static inline void *_hashtable_einit(size_t *HASHTABLE_RESTRICT num_buckets, size_t num,
    size_t bucket_size, size_t *HASHTABLE_RESTRICT num_values)
{
    int err;
    void *ret = _hashtable_init(num_buckets, num, bucket_size, num_values,
        &err);
    if (err)
        hashtable_panic();
    return ret;
}

static inline void *_hashtable_einsert(unsigned char *HASHTABLE_RESTRICT buckets,
    size_t *HASHTABLE_RESTRICT num_buckets, size_t *HASHTABLE_RESTRICT num_values,
    size_t bucket_size, size_t key_off1, size_t value_off1, size_t hash_off1,
    void *HASHTABLE_RESTRICT key, size_t key_size, size_t hash, void *HASHTABLE_RESTRICT value,
    size_t value_size,
    int (*compare_keys)(const void *a, const void *b, size_t size),
    int (*copy_key)(void *dst, const void *src, size_t size))
{
    int err;
    void *ret = _hashtable_insert(&err, buckets, num_buckets, num_values,
        bucket_size, key_off1, value_off1, hash_off1, key, key_size, hash,
        value, value_size, compare_keys, copy_key);
    if (err)
        hashtable_panic();
    return ret;
}

#endif
