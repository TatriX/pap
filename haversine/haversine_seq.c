#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define len(a) (sizeof(a)/sizeof(0[a]))
#define sqr(a) ((a) * (a))
#define pi 3.14159265358979323846f

#ifdef DEBUG
#define debugf(fmt, ...) printf(fmt, __VA_ARGS__);
#else
#define debugf(...)
#endif

typedef float f32;

static inline f32
radians(f32 degrees) {
    f32 result = degrees * pi / 180.0f;
    return result;
}

static inline f32
haversine_distance(f32 x0, f32 y0, f32 x1, f32 y1, f32 radius) {
    f32 dY = radians(y1 - y0);
    f32 dX = radians(x1 - x0);
    y0 = radians(y0);
    y1 = radians(y1);

    f32 root = (sqr(sinf(dY/2.0f))) + cosf(y0) * cosf(y1) * sqr(sin(dX/2));
    f32 result = 2.0f * radius * asin(sqrt(root));
    return result;
}

int
main() {
    const char *inpath = "data.json";
    printf("Reading from %s\n", inpath);
    FILE *infile = fopen(inpath, "r");
    assert(infile);

    int count = 0;
    f32 sum = 0;
    f32 earth_radius_km = 6371.0f;

    clock_t start_time = clock();

    const int npoints = 10'000'000;
    f32 *points = malloc(npoints * 4 * sizeof(f32));
    assert(points);

    // NOTE: skip to the first record
    {
        char buf[256];
        int nread = fread(buf, 1, len(buf), infile);
        assert(nread > 0);

        for (int i = 1; i < nread; i++) {
            if (buf[i] == '{') {
                debugf("Found { at %d: %.*s\n", i, 10, buf + i);
                fseek(infile, i, SEEK_SET);
                break;
            }
        }
    }


    while (true) {
        f32 *x0 = points + 4*count + 0;
        f32 *y0 = points + 4*count + 1;
        f32 *x1 = points + 4*count + 2;
        f32 *y1 = points + 4*count + 3;
        int nscanned = fscanf(infile, "{\"x0\": %f, \"y0\": %f, \"x1\": %f, \"y1\": %f}", x0, y0, x1, y1);
        if (nscanned == 0) {
            break;
        }

        debugf("Got %f %f, %f %f\n", x0, y0, x1, y1);

        fscanf(infile, ",\n");
        assert(count++ < npoints);
    }
    assert(count == npoints);

    clock_t mid_time = clock();

    for (int i = 0; i < npoints; i++) {
        f32 x0 = points[4*i + 0];
        f32 y0 = points[4*i + 1];
        f32 x1 = points[4*i + 2];
        f32 y1 = points[4*i + 3];
        sum += haversine_distance(x0, y0, x1, y1, earth_radius_km);
    }

    clock_t end_time = clock();

    fclose(infile);

    f32 avg = sum / count;
    double time_spent = (double)(end_time - start_time)/CLOCKS_PER_SEC;
    printf("Avg of %d records: %f\n", count, avg);
    printf("Input = %.2f seconds\n", (double)(mid_time - start_time)/CLOCKS_PER_SEC);
    printf("Math = %.2f seconds\n", (double)(end_time - mid_time)/CLOCKS_PER_SEC);
    printf("Total = %.2f seconds\n", time_spent);
    printf("Throughput = %.f haversines/second\n", count/time_spent);
}
