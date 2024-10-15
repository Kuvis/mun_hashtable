#include <string.h>
#include <stdlib.h>
#include "../hashtable.h"

size_t compute_hash(const void *key, size_t size)
{
    (void)size;
    return hashtable_hash(*(const char**)key, strlen(*(const char**)key));
}

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

/* Type and function definition */
hashtable_define_ext(str_int_table, const char *, int, compute_hash,
    compare_keys, copy_key, free_key);

int main(int argc, char **argv)
{
    /* Initialization */
    struct str_int_table table;
    if (str_int_table_init(&table, 8))
        return -1;

    /* Insertion */
    if (str_int_table_insert(&table, "one", 1))
        return -1;

    /* Finding */
    {
        int *value = str_int_table_find(&table, "one");
        if (value) {
            /* ... Entry was found ... */
        }
    }

    /* Iteration */
    {
        const char  *key;
        int         value;
        hashtable_for_each_pair(table, key, value) {
            /* Do something with key and value */
        }
    }

    /* Erasing */
    str_int_table_erase(&table, "one");

    /* Destroying */
    str_int_table_destroy(&table);

    return 0;
}
