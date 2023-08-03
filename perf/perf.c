#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <x86intrin.h>

typedef uint64_t u64;
typedef double f64;

u64 Nanoseconds = 1000000000;
u64 Milliseconds = 1000000;

// nanoseconds
static u64
now(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * Nanoseconds + tp.tv_nsec;
}

static f64
estimate_cpu_freq(u64 time_to_wait) {
    u64 start_cycles = _rdtsc();
    for (u64 started = now(); now() - started < time_to_wait; ) {
        // busy wait
    }
    u64 end_cycles = _rdtsc();
    u64 elapsed_cycles = end_cycles - start_cycles;
    f64 result = ((f64)elapsed_cycles / (f64)time_to_wait);
    return result;
}


int main(int argc, char *argv[]) {
    u64 time_to_wait = 100 * Milliseconds;

    if (argc > 1) {
        time_to_wait = atol(argv[1]) * Milliseconds;
    }
    printf("Waiting for %ldms\n", time_to_wait / Milliseconds);

    f64 cpu_freq = estimate_cpu_freq(time_to_wait);

    printf("Freq: %fGhz\n", cpu_freq);

    return 0;
}
