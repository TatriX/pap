#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <x86intrin.h>

typedef uint64_t u64;
typedef double f64;
typedef f64 t64;

static t64
now(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec + tp.tv_nsec * 1e-9;
}


int main(int argc, char *argv[]) {
    printf("Start: %f:\n", now());

    u64 start_cycles = _rdtsc();
    for (t64 started = now(); now() - started < 1.0; ) {
       // busy wait
    }
    u64 end_cycles = _rdtsc();

    u64 elapsed_cycles = end_cycles - start_cycles;
    printf("Cycles elapsed: %ld\n", elapsed_cycles);
    printf("Freq: %fGhz\n", (elapsed_cycles / 1e9));

    return 0;
}
