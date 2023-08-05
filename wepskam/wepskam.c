#include <stdio.h>
#include <unistd.h>
#include <x86intrin.h>
#include <locale.h>
#include <time.h>

typedef void (*proc)(void);

static void
mat_set(const int N, double mat[][N], double value) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            mat[i][j] = value;
        }
    }
}

#define N 1000
double mul1[N][N];
double mul2[N][N];
double res[N][N];
double tmp[N][N];

static void
benchmark(proc body) {
    struct timespec started, ended;
    clock_gettime(CLOCK_MONOTONIC, &started);
    long started_cycles = __rdtsc();

    body();
    long ended_cycles = __rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &ended);

    long elapsed_cycles = ended_cycles - started_cycles;

    printf("Cycles elapsed: %'ld\n", elapsed_cycles);
    double elapsed = (ended.tv_sec * 1e9 + ended.tv_nsec) - (started.tv_sec * 1e9 + started.tv_nsec);
    printf("Time elapsed: %'fs\n", elapsed * 1e-9);
}

static void
naive(void) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                res[i][j] += mul1[i][k] * mul2[k][j];
            }
        }
    }
}

static void
transposed(void) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            tmp[i][j] = mul2[j][i];
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                res[i][j] += mul1[i][k] * tmp[j][k];
            }
        }
    }
}

static void
crazy(void) {
#define CLS 64 // getconf LEVEL1_DCACHE_LINESIZE
#define SM (CLS / sizeof (double))
    int i, i2, j, j2, k, k2;
    double *rmul1, *rmul2, *rres;
    for (i = 0; i < N; i += SM)
        for (j = 0; j < N; j += SM)
            for (k = 0; k < N; k += SM)
                for (i2 = 0, rres = &res[i][j],
                         rmul1 = &mul1[i][k]; i2 < SM;
                     ++i2, rres += N, rmul1 += N)
                    for (k2 = 0, rmul2 = &mul2[k][j];
                         k2 < SM; ++k2, rmul2 += N)
                        for (j2 = 0; j2 < SM; ++j2)
                            rres[j2] += rmul1[k2] * rmul2[j2];
}

int main(int argc, char *argv[]) {
    long l1size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("l1size: %ldbytes\n", l1size);

    // NOTE: teach printf to use thousands separator
    setlocale(LC_NUMERIC, "");
    struct lconv *ptrLocale = localeconv();
    ptrLocale->thousands_sep = "'";

    // NOTE naive matrix multiplication
    mat_set(N, mul1, 42);
    mat_set(N, mul2, 13);
    mat_set(N, res, 0);

    printf("# Naive\n");
    benchmark(naive);

    printf("# Transposed\n");
    benchmark(transposed);

    printf("# Crazy\n");
    benchmark(crazy);
}
