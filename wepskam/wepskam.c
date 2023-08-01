#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    long l1size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("l1size: %ldbytes\n", l1size);
}
