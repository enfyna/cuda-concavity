#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "raylib.h"

__global__ void calculate_angles(char* dest, size_t count, Vector2* points)
{
    Vector2 l, p, r;

    l = points[(blockIdx.x + count - 1) % count];
    p = points[blockIdx.x];
    r = points[(blockIdx.x + 1) % count];

    // Vector2Subtract(l, p);
    Vector2 ba = (Vector2) { .x = l.x - p.x, .y = l.y - p.y };
    // Vector2Subtract(r, p);
    Vector2 bc = (Vector2) { .x = r.x - p.x, .y = r.y - p.y };

    float cross = ba.x * bc.y - ba.y * bc.x;

    dest[blockIdx.x] = (cross >= 0) ? 1 : -1;

    // printf("block: %u, thread: %u => %d\n"
    // "%5.1f,%5.1f|"
    // "%5.1f,%5.1f|"
    // "%5.1f,%5.1f|\n",
    // blockIdx.x, threadIdx.x, dest[blockIdx.x], l.x, l.y, p.x, p.y, r.x, r.y);
}

double get_seconds(void)
{
    struct timespec ts;
    int res = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(res != -1);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

extern "C" {

bool is_concave(size_t count, Vector2* points)
{
    cudaError_t err;

    int size = count * sizeof(Vector2);
    int size_dest = count * sizeof(char);

    Vector2* d_a;
    char* d_dest;

    double start = get_seconds();
    err = cudaMalloc(&d_a, size);
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %s\n", cudaGetErrorString(err));
        fprintf(stderr, "err: %d\n", err);
        exit(1);
    }

    err = cudaMalloc(&d_dest, size_dest);
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %s\n", cudaGetErrorString(err));
        fprintf(stderr, "err: %d\n", err);
        exit(1);
    }

    err = cudaMemcpy(d_a, points, size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        printf("cudaMemcpy failed: %s\n", cudaGetErrorString(err));
        fprintf(stderr, "err: %d\n", err);
        exit(1);
    }
    double end = get_seconds();
    printf("time allocating and copying: %lf\n", end - start);

    start = get_seconds();
    calculate_angles<<<count, 1>>>(d_dest, count, d_a);
    cudaDeviceSynchronize();
    end = get_seconds();
    printf("time calculate_angles: %lf\n", end - start);

    start = get_seconds();
    char* dest = (char*)malloc(sizeof(char) * count);
    cudaMemcpy(dest, d_dest, size_dest, cudaMemcpyDeviceToHost);

    cudaFree(d_a);
    cudaFree(d_dest);
    end = get_seconds();
    printf("time copying and freeing: %lf\n", end - start);

    for (size_t i = 1; i <= count; i++) {
        if (dest[i % count] + dest[i - 1] == 0) {
            // printf("%d\n", dest[i % count]);
            return true;
        }
        // printf("%d, ", dest[i - 1]);
    }
    // printf("\n");

    return false;
}
}
