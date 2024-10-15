#include "../hashtable.h"

hashtable_define(int_int_table, int, int);

int main(int argc, char **argv)
{
    /* Initialization */
    struct int_int_table table;
    if (int_int_table_init(&table, 8))
        return -1;

    /* Insertion */
    if (int_int_table_insert(&table, 12345, 54321))
        return -1;

    /* Finding */
    {
        int *value = int_int_table_find(&table, 12345);
        if (value) {
            /* ... Entry was found ... */
        }
    }

    /* Iteration */
    {
        int key;
        int value;
        hashtable_for_each_pair(table, key, value) {
            /* Do something with key and value */
        }
    }

    /* Erasing */
    int_int_table_erase(&table, 12345);

    /* Destroying */
    int_int_table_destroy(&table);

    return 0;
}
