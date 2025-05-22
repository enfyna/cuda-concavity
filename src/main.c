#include <assert.h>
#include <dlfcn.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "raylib.h"
#include "raymath.h"

typedef struct {
    const char* name;
    size_t count;
    Vector2* points;
} Polygon;

typedef bool(func_cuda)(size_t count, Vector2* points);

int main(int argc, char* argv[])
{
    (void)argv;

    void* handle = dlopen("./build/cuda.so", RTLD_NOW);
    func_cuda* is_concave = (func_cuda*)dlsym(handle, "is_concave");
    if (is_concave == NULL) {
        assert(false && "is_concave NOT FOUND");
    }

    Graph g = graph((Rectangle) { 5, 5, 790, 790 }, 10, RED, BLACK, DARKGRAY);
    g.st_reset.pos.x = 405;
    g.st_reset.pos.y = 380;
    g.st_reset.scale.z = 30;

    g.st_current.pos = g.st_reset.pos;
    g.st_current.scale = g.st_reset.scale;

    Polygon concave_pg = {
        .name = "Pacman",
        .count = 20,
        .points = (Vector2[]) {
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
        }
    };

    Polygon convex_pg = {
        .name = "Circle",
        .count = 20,
        .points = (Vector2[]) {
            { 10.0, 0.0 },
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
        }
    };

    Polygon pg;
    if (argc == 2) {
        pg = concave_pg;
    } else {
        pg = convex_pg;
    }

    Vector2* points = malloc(sizeof(Vector2) * pg.count);
    memcpy(points, pg.points, sizeof(Vector2) * pg.count);

    bool concavity = is_concave(pg.count, pg.points);

    char buf_concv[128];
    sprintf(buf_concv, "Is %s concave ? %s", pg.name, (concavity) ? "True" : "False");
    char buf_angle[128];

    printf("%s\n", buf_concv);

    // exit(0);

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

    dlclose(handle);
    graph_free(&g);

    return 0;
}
