#define _DEFAULT_SOURCE 1
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>

#include "lib/battleshipslib.h"

typedef struct {
    long cycles;
    struct timespec time;
} Timer;

static inline void get_time(Timer *timer)
{
    timer->cycles = __rdtsc();
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timer->time) != 0)
    {
        exit(1);
    }
}

void print_time(Timer start, Timer end)
{
    long cycles = end.cycles - start.cycles;
    long nanoseconds = ((end.time.tv_sec - start.time.tv_sec) * (long)1e9) + (
        end.time.tv_nsec - start.time.tv_nsec
    );
    printf("%ld cycles, %ld ns\n", cycles, nanoseconds);
}

int main()
{

    return 0;
}
