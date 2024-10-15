#include "../hashtable.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv)
{
    /* Initialization */
    hashtable(uint64_t, uint32_t) table;
    int err;
    hashtable_init(table, 8, &err);
    if (err)
        return -1;

    /* Insertion */
    {
        uint64_t    key     = 12345;
        uint32_t    value   = 54321;
        size_t      hash    = hashtable_hash(&key, sizeof(key));
        hashtable_insert(table, key, hash, value, &err);
        if (err)
            return -1;
    }

    /* Finding */
    {
        uint64_t    key     = 12345;
        size_t      hash    = hashtable_hash(&key, sizeof(key));
        uint32_t    *value  = hashtable_find(table, key, hash);
        if (value) {
            /* ... Entry was found ... */
        }
    }

    /* Iteration */
    {
        uint64_t key;
        uint32_t value;
        hashtable_for_each_pair(table, key, value) {
            /* Do something with key and value */
        }
    }

    /* Erasing */
    {
        uint64_t    key  = 12345;
        size_t      hash = hashtable_hash(&key, sizeof(key));
        hashtable_erase(table, key, hash);
    }

    /* Destroying */
    hashtable_destroy(table, NULL);

    return 0;
}
