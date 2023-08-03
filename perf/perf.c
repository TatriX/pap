#include <assert.h>
#include <stdio.h>

#include "perf.h"

int main(int argc, char *argv[]) {
    u64 time_to_wait = 100 * Milliseconds;

    if (argc > 1) {
        time_to_wait = atol(argv[1]) * Milliseconds;
    }
    printf("Waiting for %ldms\n", time_to_wait / Milliseconds);

    u64 cpu_freq = estimate_cpu_freq(time_to_wait);

    printf("Freq: %fGhz\n", cpu_freq / Ghz);

    return 0;
}
