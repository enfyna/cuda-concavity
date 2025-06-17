#define _XOPEN_SOURCE 600
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <assert.h>
#include <dlfcn.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "graph.h"
#include "raylib.h"
#include "raymath.h"

#define CIRCLE_MAX (3600000 * 7)

typedef struct {
    const char* name;
    size_t count;
    Vector2* points;
} Polygon;

typedef bool(f_concavity)(size_t count, Vector2* points);

double get_seconds(void)
{
    struct timespec ts;
    int res = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(res != -1);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(int argc, char* argv[])
{
    (void)argv;

    void* h_cuda = dlopen("./build/cuda.so", RTLD_NOW);
    f_concavity* is_concave_cuda = (f_concavity*)dlsym(h_cuda, "is_concave");
    if (is_concave_cuda == NULL) {
        assert(false && "is_concave NOT FOUND");
    }

    void* h_sync = dlopen("./build/sync.so", RTLD_NOW);
    f_concavity* is_concave_sync = (f_concavity*)dlsym(h_sync, "is_concave");
    if (is_concave_sync == NULL) {
        assert(false && "is_concave NOT FOUND");
    }

    Graph g = graph((Rectangle) { 5, 5, 790, 790 }, 10, RED, BLACK, DARKGRAY);
    g.st_reset.pos.x = 405;
    g.st_reset.pos.y = 380;
    g.st_reset.scale.z = 30;

    g.st_current.pos = g.st_reset.pos;
    g.st_current.scale = g.st_reset.scale;

    g.show_legend = false;

    Polygon pg;
    if (argc == 2) {
        pg.name = "Pacman";
        pg.count = 20;
        pg.points = (Vector2[]) {
            { 5.0, 0.0 },
            { 9.51, 3.09 },
            { 8.09, 5.88 },
            { 5.88, 8.09 },
            { 3.09, 9.51 },
            { 0.0, 10.0 },
            { -3.09, 9.51 },
            { -5.88, 8.09 },
            { -8.09, 5.88 },
            { -9.51, 3.09 },
            { -10.0, 0.0 },
            { -9.51, -3.09 },
            { -8.09, -5.88 },
            { -5.88, -8.09 },
            { -3.09, -9.51 },
            { 0.0, -10.0 },
            { 3.09, -9.51 },
            { 5.88, -8.09 },
            { 8.09, -5.88 },
            { 9.51, -3.09 },
        };
    } else {
        pg.name = "Circle";
        pg.count = CIRCLE_MAX;

        pg.points = malloc(sizeof(Vector2) * pg.count);

        long double deg = 360 / (long double)(pg.count - 1);
        long double radius = 20;
        size_t i = 0;

        for (long double cdeg = 0; cdeg <= 360; cdeg += deg) {
            long double y = sin(cdeg * DEG2RAD) * radius;
            long double x = cos(cdeg * DEG2RAD) * radius;
            pg.points[i++] = (Vector2) { x, y };
        }

        printf("count: %zu\n", pg.count);
        assert(i == pg.count);
    }

    double start = get_seconds();
    bool concavity_serial = is_concave_sync(pg.count, pg.points);
    double end = get_seconds();

    double start_paralell = get_seconds();
    bool concavity = is_concave_cuda(pg.count, pg.points);
    double end_paralell = get_seconds();

    assert(concavity == concavity_serial);

    printf("parallel time: total: %f\n", end_paralell - start_paralell);
    printf("serial time: total: %f\n", end - start);

    char buf_concv[128];
    sprintf(buf_concv, "Is %s concave ? %s", pg.name, (concavity) ? "Yes" : "No");
    char buf_angle[128];

    printf("%s\n", buf_concv);

    exit(0);

    Line* es[pg.count];
    for (size_t i = 0; i < pg.count; i++) {
        char* buf = malloc(sizeof(char) * 8);
        sprintf(buf, "L%2zu", i);
        es[i] = line(&g, 101, buf,
            (Color) { .a = 255, .r = 250, .g = 250, .b = i * 10 + 50 });
    }

    for (size_t i = 0; i < pg.count; i++) {
        Line* line = es[i];

        Vector2 start = pg.points[i];
        Vector2 end = pg.points[(i + 1) % pg.count];

        Vector2 stride = Vector2Subtract(end, start);
        stride.x /= 100.0;
        stride.y /= 100.0;

        Vector2 current = start;
        for (size_t i = 0; i < 101; i++) {
            line->points[i] = current;
            current = Vector2Add(current, stride);
        }
    }

    Vector2 l, p, r;

    size_t c = 0;
    double acc = 10.0;

    SetTargetFPS(144);
    InitWindow(800, 800, "Paralleling All Over The Place");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (IsKeyPressed(KEY_E)) {
            break;
        }

        if (IsKeyPressed(KEY_P)) {
            graph_print_position(&g);
        }

        acc += GetFrameTime();

        if (acc > 0.4) {
            acc = .0;
            c++;

            l = pg.points[(c + pg.count - 1) % pg.count];
            p = pg.points[c % pg.count];
            r = pg.points[(c + 1) % pg.count];

            Vector2 ba = Vector2Subtract(l, p);
            Vector2 bc = Vector2Subtract(r, p);
            float dot = Vector2DotProduct(ba, bc);

            ba = Vector2Multiply(ba, ba);
            bc = Vector2Multiply(bc, bc);

            float mba = sqrt(ba.x + ba.y);
            float mbc = sqrt(bc.x + bc.y);

            double rad = dot / (mba * mbc);
            double deg = acos(rad) * 180 / PI;
            sprintf(buf_angle, "Angle : %lf\n", deg);
        }

        graph_update(&g);

        graph_draw(&g);

        graph_draw_point(&g, p, 7, RED);
        graph_draw_point(&g, l, 7, ORANGE);
        graph_draw_point(&g, r, 7, PURPLE);

        DrawText(buf_angle, g.bound.x, g.bound.height + g.bound.y - 48, 24, WHITE);
        DrawText(buf_concv, g.bound.x, g.bound.height + g.bound.y - 24, 24, WHITE);

        EndDrawing();
    }

    dlclose(h_cuda);
    dlclose(h_sync);
    graph_free(&g);

    return 0;
}
