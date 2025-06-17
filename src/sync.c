#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"

bool is_concave(size_t count, Vector2* points)
{
    int* cs = malloc(sizeof(int) * count);
    Vector2 l, p, r;

    for (size_t i = 0; i < count; i++) {
        l = points[(i + count - 1) % count];
        p = points[i % count];
        r = points[(i + 1) % count];

        Vector2 ba = Vector2Subtract(l, p);
        Vector2 bc = Vector2Subtract(r, p);

        double cross = (ba.x * bc.y - ba.y * bc.x >= 0) ? 1 : -1;

        cs[i] = cross >= 0 ? 1 : -1;
    }

    bool res = false;
    for (size_t i = 1; i <= count; i++) {
        if (cs[i % count] + cs[i - 1] == 0) {
            res = true;
        }
    }
    free(cs);
    return res;
}
