#include <string.h>
#include <stdlib.h>
#include "../hashtable.h"

int compare_keys(const void *a, const void *b, size_t size)
    {return strcmp(*(const char**)a, *(const char**)b);}

int copy_key(void *dst, const void *src, size_t size)
{
    size_t len = strlen(*(const char**)src);
    *(char**)dst = malloc(len + 1);
    if (!*(char**)dst)
        return 1;
    memcpy(*(char**)dst, *(const char**)src, len + 1);
    return 0;
}

void free_key(void *key)
    {free(*(char**)key);}

int main(int argc, char **arg)
{
    /* Initialization */
    hashtable(char *, int) table;
    int err;
    hashtable_init(table, 8, &err);
    if (err)
        return -1;

    /* Insertion */
    {
        char    *key    = "one";
        size_t  hash    = hashtable_hash(key, strlen(key));
        int     value   = 1;
        hashtable_insert_ext(table, key, hash, value, compare_keys, copy_key,
            &err);
        if (err)
            return -1;
    }

    /* Finding */
    {
        char    *key = "one";
        size_t  hash = hashtable_hash(key, strlen(key));
        int     *one = hashtable_find_ext(table, key, hash, compare_keys);
        if (one) {
            /* ... Value was found ... */
        }
    }

    /* Iteration */
    {
        char    *key;
        int     value;
        hashtable_for_each_pair(table, key, value) {
            /* Do something with key and value */
        }
    }

    /* Erasing */
    {
        char    *key = "one";
        size_t  hash = hashtable_hash(key, strlen(key));
        hashtable_erase_ext(table, key, hash, compare_keys, free_key);
    }

    /* Destroying */
    hashtable_destroy(table, free_key);

    return 0;
}
