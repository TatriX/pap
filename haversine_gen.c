#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef float f32;

// between -90 and 90
static inline f32
rand_latitude(void) {
    f32 result = -90.0f + (180.0f * (f32)rand() / RAND_MAX);
    return result;
}

// between -180 and 180
static inline f32
rand_longitude(void) {
    f32 result = -180.0f + (360.0f * (f32)rand() / RAND_MAX);
    return result;
}


int
main() {
    srand(42);

    int npoints = 10'000'000;
    const char *outpath = "data.json";
    printf("Generating %d points to %s\n", npoints, outpath);
    FILE *outfile = fopen(outpath, "w");
    assert(outfile);

    /* outfile = stdout; */
    fprintf(outfile, "{\n    pairs: [\n");

    for (int i = 0; i < npoints; i++) {
        f32 x0 = rand_latitude();
        f32 y0 = rand_longitude();
        f32 x1 = rand_latitude();
        f32 y1 = rand_longitude();
        fprintf(outfile, "        {\"x0\": %f, \"y0\": %f, \"x1\": %f, \"y1\": %f}", x0, y0, x1, y1);
        if (i < npoints - 1)
            fprintf(outfile, ",\n");
    }
    fprintf(outfile, "\n    ]\n}");

    fclose(outfile);
}
