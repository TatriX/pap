#pragma once

#include <time.h>
#include <x86intrin.h>

#include "../basic.h"

const u64 Seconds = 1000000000;
const u64 Milliseconds = 1000000;

const f64 Ghz = 1000000000;

static inline u64
rdtsc(void) {
    return _rdtsc();
}

// nanoseconds
u64
now(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * Seconds + tp.tv_nsec;
}

u64
estimate_cpu_freq(u64 ns_to_wait) {
    if (ns_to_wait == 0) {
        ns_to_wait = 100 * Milliseconds;
    }
    u64 start_cycles = rdtsc();
    for (u64 started = now(); now() - started < ns_to_wait; ) {
        // busy wait
    }
    u64 end_cycles = rdtsc();
    u64 elapsed_cycles = end_cycles - start_cycles;
    u64 result = (elapsed_cycles * Seconds / ns_to_wait);
    return result;
}
