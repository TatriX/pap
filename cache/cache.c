#include <stdio.h>
#include <string.h>

#include "../perf/perf.h"

char src[1 << 20];
char dst[1 << 20];

int main(int argc, char *argv[]) {
    begin_profile();

    for (int n = 0; n < 128; n++) {
        if (argc > 1) {
            memcpy(src, dst, sizeof(src));
        } else {
            // NOTE: with -O2 this compiles to memcpy
            for (int i = 0; i < sizeof(src); i++) {
                dst[i] = src[i];
            }
        }
    }

    end_and_print_profile();
    return 0;
}
