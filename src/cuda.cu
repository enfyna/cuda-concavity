#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "raylib.h"

__global__ void calculate_angles(int* dest, size_t count, Vector2* points)
{
    Vector2 l, p, r;
    size_t c = blockIdx.x;

    l = points[(c + count - 1) % count];
    p = points[c % count];
    r = points[(c + 1) % count];
    Vector2 ps[] = { l, p, r };

    // Vector2Subtract(l, p);
    Vector2 ba = (Vector2) { .x = l.x - p.x, .y = l.y - p.y };
    // Vector2Subtract(r, p);
    Vector2 bc = (Vector2) { .x = r.x - p.x, .y = r.y - p.y };

    float cross = ba.x * bc.y - ba.y * bc.x;

    dest[blockIdx.x] = (cross >= 0) ? 1 : -1;
    printf("block: %u, thread: %u => %d\n"
           "%5.1f,%5.1f|"
           "%5.1f,%5.1f|"
           "%5.1f,%5.1f|\n",
        blockIdx.x, threadIdx.x, dest[blockIdx.x], ps[0].x, ps[0].y, ps[1].x, ps[1].y, ps[2].x, ps[2].y);
}

extern "C" {

bool is_concave(size_t count, Vector2* points)
{
    cudaError_t err;

    printf("%zu:\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("%f, %f\n", points[i].x, points[i].y);
    }

    int size = count * sizeof(Vector2);
    int size_dest = count * sizeof(int);

    Vector2* d_a;
    err = cudaMalloc(&d_a, size);
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %s\n", cudaGetErrorString(err));
        fprintf(stderr, "err: %d\n", err);
        exit(1);
    }

    int* d_dest;
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

    calculate_angles<<<count, 1>>>(d_dest, count, d_a);
    cudaDeviceSynchronize();

    int dest[count];
    cudaMemcpy(dest, d_dest, size_dest, cudaMemcpyDeviceToHost);

    cudaFree(d_a);
    cudaFree(d_dest);

    for (size_t i = 1; i <= count; i++) {
        if (dest[i % count] + dest[i - 1] == 0) {
            printf("\n");
            return true;
        }
        printf("%d,", dest[i % count]);
    }
    printf("\n");

    return false;
}
}
