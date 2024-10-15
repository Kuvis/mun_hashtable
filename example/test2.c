#include "../hashtable.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

typedef struct {
    uint64_t    sec;
    uint64_t    msec;
} sys_time_t;

int get_monotonic_time(sys_time_t *ret_time);
void init(void);
long long unsigned print_times(sys_time_t *start_time, sys_time_t *end_time);

static LARGE_INTEGER _freq = {0};

int main(int argc, char **argv)
{
    sys_time_t  start_time, end_time;
    uint32_t    num_values = 10000000;
    printf("Num values: %u\n", num_values);
    init();
    /* Init */
    puts("# Initialization");
    get_monotonic_time(&start_time);
    hashtable(uint32_t, int) table;
    int err;
    hashtable_init(table, 8, &err);
    get_monotonic_time(&end_time);
    assert(!err);
    printf("Initialization took %llu ms.\n",
        print_times(&start_time, &end_time));
    /* Insertion */
    puts("# Insertion");
    get_monotonic_time(&start_time);
    for (uint32_t i = 0; i < num_values; ++i) {
        int v = i;
        hashtable_insert(table, i, hashtable_hash(&i, sizeof(i)), v, &err);
    }
    get_monotonic_time(&end_time);
    printf("Insertion took %llu ms.\n", print_times(&start_time, &end_time));
    puts("# Iteration");
    get_monotonic_time(&start_time);
    /* Iteration */
    uint32_t num_iterations = 0;
    uint32_t    k;
    int         v;
    hashtable_for_each_pair(table, k, v)
        num_iterations++;
    get_monotonic_time(&end_time);
    printf("Iteration took %llu ms, num iterations: %u.\n", print_times(&start_time, &end_time), num_iterations);
    /* Finding */
    puts("# Finding");
    get_monotonic_time(&start_time);
    for (uint32_t i = 0; i < num_values; ++i)
        hashtable_find(table, i, hashtable_hash(&i, sizeof(i)));
    get_monotonic_time(&end_time);
    printf("Finding took %llu ms.\n", print_times(&start_time, &end_time));
    /* Erasing */
    puts("# Erasing");
    get_monotonic_time(&start_time);
    for (uint32_t i = 0; i < num_values; ++i)
        hashtable_erase(table, i, hashtable_hash(&i, sizeof(i)));
    get_monotonic_time(&end_time);
    printf("Erasing took %llu ms.\n", print_times(&start_time, &end_time));
    /* Destruction */
    puts("# Destruction");
    get_monotonic_time(&start_time);
    hashtable_destroy(table, 0);
    get_monotonic_time(&end_time);
    printf("Destruction took %llu ms.\n", print_times(&start_time, &end_time));
    return 0;
}

void init(void)
{
    QueryPerformanceFrequency(&_freq);
}

long long unsigned print_times(sys_time_t *start_time, sys_time_t *end_time)
{
    long long unsigned start_ms  = (long long unsigned)start_time->sec * 1000ULL + (long long unsigned)start_time->msec;
    long long unsigned end_ms = (long long unsigned)end_time->sec * 1000ULL + (long long unsigned)end_time->msec;
    return end_ms - start_ms;
}

int get_monotonic_time(sys_time_t *ret_time)
{
    LARGE_INTEGER num_ticks;
    if (QueryPerformanceCounter(&num_ticks) == 0)
        return 2;
    num_ticks.QuadPart = num_ticks.QuadPart / (_freq.QuadPart / 1000);
    ret_time->sec   = (uint64_t)(num_ticks.QuadPart / 1000);
    ret_time->msec  = (uint64_t)(num_ticks.QuadPart % 1000);
    return 0;
}

