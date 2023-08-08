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

struct _profile_block {
    u64 begin_cycles;
    u64 end_cycles;
    const char *label;
};

    struct {
    u64 start_cycles;
    struct _profile_block blocks[128];
    u16 nblocks;
} _profile;

static void
begin_profile(void) {
    _profile.start_cycles = rdtsc();
}

static void
begin_profile_block(const char *label) {
    _profile.blocks[_profile.nblocks].begin_cycles = rdtsc();
    _profile.blocks[_profile.nblocks].label = label;
}

static void
end_profile_block(void) {
    _profile.blocks[_profile.nblocks++].end_cycles = rdtsc();
}

static void
end_and_print_profile(void) {
    assert(_profile.nblocks < len(_profile.blocks));
    u64 end_cycles = rdtsc();

    u64 cpu_freq = estimate_cpu_freq(0);

    u64 total_cycles = (end_cycles - _profile.start_cycles);
    f64 time_spent = (f64)total_cycles/cpu_freq;
    printf("Total Time: %.2fs (Est. CPU Freq: %.2fGz)\n", time_spent, cpu_freq/1e9);

    for (int i = 0; i < _profile.nblocks; i++) {
        struct _profile_block *block = _profile.blocks + i;
        u64 elapsed_cycles = block->end_cycles - block->begin_cycles;
        f64 percentage = (f64)elapsed_cycles / (f64) total_cycles * 100.0;
        printf("%20s :: %12ld (%.2f%%)\n", block->label, elapsed_cycles, percentage);
    }
}
