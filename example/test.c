#include "../hashtable.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef long long unsigned llu_t;

typedef struct {
    uint64_t sec;
    uint64_t msec;
} sys_time_t;

typedef struct {
    int found;
    int val;
} entry_t;

int get_monotonic_time(sys_time_t *ret_time)
{
    struct timespec timespec;
    if (clock_gettime(CLOCK_MONOTONIC, &timespec))
        return 2;
    ret_time->sec   = timespec.tv_sec;
    ret_time->msec  = timespec.tv_nsec / 1000000;
    return 0;
}

int main(int argc, char **argv)
{
    sys_time_t start_time, end_time;
    if (get_monotonic_time(&start_time))
        return 1;
    hashtable(uint32_t, int) table;
    hashtable_init(table, 8, 0);
    uint32_t num_items = 1000000;
    for (uint32_t i = 0; i < num_items; ++i) {
        int v = (int)i;
        int err;
        hashtable_insert(table, i, hashtable_hash(&i, sizeof(i)), v, &err);
        assert(!err);
    }
    uint32_t    num_incorrect   = 0;
    int         last            = -1;
    for (uint32_t i = 0; i < num_items; ++i) {
        int *value  = hashtable_find(table, i, hashtable_hash(&i, sizeof(i)));
        assert(value);
        int diff    = *value - last;
        diff = diff < 0 ? -diff : diff;
        if (diff != 1)
            num_incorrect++;
        last = *value;
    }
    uint32_t    k;
    int         v;
    size_t      num_iterations  = 0;
    entry_t     *entries        = calloc(num_items, sizeof(entry_t));
    hashtable_for_each_pair(table, k, v) {
        assert(k < num_items);
        assert((int)k == v);
        entry_t *entry = &entries[k];
        entry->found    = 1;
        entry->val      = v;
        num_iterations++;
    }
    assert(num_iterations == num_items);
    for (int i = 0; i < num_iterations; ++i) {
        assert(entries[i].found);
    }
    uint32_t num_failed_erases = 0;
    for (uint32_t i = 0; i < num_items; ++i) {
        size_t num_values = hashtable_num_values(table);
        hashtable_erase(table, i, hashtable_hash(&i, sizeof(i)));
        if (hashtable_num_values(table) == num_values)
            num_failed_erases++;
        assert(!hashtable_exists(table, i, hashtable_hash(&i, sizeof(i))));
    }
    size_t num_buckets = hashtable_num_buckets(table);
    hashtable_destroy(table, 0);
    if (get_monotonic_time(&end_time))
        return 2;
    llu_t start_ms  = (llu_t)start_time.sec * 1000ULL + (llu_t)start_time.msec;
    llu_t end_ms    = (llu_t)end_time.sec * 1000ULL + (llu_t)end_time.msec;
    printf("Number of inserts: %u\n"
        "Number of incorrect entries: %u\n"
        "Number of failed erases: %u\n"
        "Number of buckets: %lu\n"
        "Number of for-each iterations: %lu\n", num_items, num_incorrect,
        num_failed_erases, num_buckets, num_iterations);
    printf("Time: %llu ms\n", end_ms - start_ms);
    hashtable_clear(table, 0);
    return 0;
}
