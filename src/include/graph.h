#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stddef.h>

#include "raylib.h"

#define X_AXIS 0
#define Y_AXIS 1

typedef struct {
    const char* name;
    Color color;
    size_t count;
    Vector2 points[];
} Line;

typedef struct {
    Line** items;
    size_t count;
    size_t capacity;
} Lines;

typedef struct {
    Vector2 pos;
    Vector3 scale;
} Stance;

typedef struct {
    Rectangle bound;
    Stance st_current;
    Stance st_reset;
    bool show_legend;
    Lines lines;
    Color c_background;
    Color c_border;
    Color c_grid;
    Font font;
    int font_size;
    double grid_margin;
} Graph;

Graph graph(Rectangle bounds, double grid_margin, Color border, Color background, Color grid);
void graph_update(Graph* g);
void graph_draw(Graph* g);
void graph_draw_grid(Graph* g);
void graph_draw_border(Graph* g);
void graph_draw_lines(Graph* g);
void graph_draw_relative_line(Graph* g, int type, int offset, Color color);
void graph_draw_line_value_at_x(Graph* g, double pos);
void graph_draw_line_value_at_mouse(Graph* g);
void graph_draw_point(Graph* g, Vector2 p, int size, Color color);
void graph_print_position(Graph* g);
void graph_zoom(Graph* g, double zoom, double delta);
void graph_free(Graph* g);

Line* line(Graph* g, int count, const char* name, Color color);
#endif // GRAPH_H
