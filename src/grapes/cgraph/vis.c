#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "vis.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

#include "cgraph.h"

#define GRAPES_NODE_SHAPE_CIRCLE 0

typedef struct Point2d {
    double x;
    double y;
} Point2d;

typedef struct NodeOptions {
    // svg
    char   *shape;
    char   *stroke;
    char   *fill;
    uint8_t stroke_width;
    // custom
    double shape_size;
} NodeOptions;

typedef struct EdgeOptions {
    // svg
    char   *stroke;
    uint8_t stroke_width;
} EdgeOptions;

Point2d *
Layout_alloc(Py_ssize_t node_count)
{
    Point2d *layout = malloc(sizeof(*layout) * node_count);
    if (layout == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate layout");
        return NULL;
    }
    return layout;
}

NodeOptions *
NodeOptions_alloc(Py_ssize_t node_count)
{
    NodeOptions *node_options = malloc(sizeof(*node_options) * node_count);
    if (node_options == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate node_options");
        return NULL;
    }

    for (Py_ssize_t i = 0; i < node_count; ++i) {
        node_options[i].shape = "circle";
        node_options[i].stroke = "black";
        node_options[i].fill = "transparent";
        node_options[i].stroke_width = 2;
        node_options[i].shape_size = 300;
    }

    return node_options;
}

EdgeOptions **
EdgeOptions_alloc(Py_ssize_t *neighbor_count, Py_ssize_t node_count)
{
    EdgeOptions **edge_options = malloc(sizeof(*edge_options) * node_count);
    if (edge_options == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate edge_options");
        return NULL;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        edge_options[i] = malloc(sizeof(*edge_options[i]) * neighbor_count[i]);
        if (edge_options[i] == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Failed to allocate edge_options[i]");
            return NULL;
        }
    }

    for (Py_ssize_t i = 0; i < node_count; ++i) {
        for (Py_ssize_t j = 0; j < neighbor_count[i]; ++j) {
            edge_options[i][j].stroke = "black";
            edge_options[i][j].stroke_width = 2;
        }
    }

    return edge_options;
}

void
Vis_free(Point2d *layout, NodeOptions *node_options,
         EdgeOptions **edge_options, Py_ssize_t node_count)
{
    free(layout);
    layout = NULL;
    free(node_options);
    node_options = NULL;
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        free(edge_options[i]);
        edge_options[i] = NULL;
    }
    free(edge_options);
    edge_options = NULL;
}

void
layout_circular(Point2d *layout, Py_ssize_t begin, Py_ssize_t end,
                double radius, double theta0, double cx, double cy)
{
    for (Py_ssize_t i = begin; i < end; ++i) {
        double theta = ((double) i / (end - begin)) * 2 * M_PI + theta0;
        layout[i].x = radius * cos(theta) + cx;
        layout[i].y = radius * sin(theta) + cy;
    }
    return;
}

void
write_svg(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
          Py_ssize_t node_count, char *filename, Point2d *layout,
          uint16_t viewbox_ul_x, uint16_t viewbox_ul_y, uint16_t viewbox_dr_x,
          uint16_t viewbox_dr_y, NodeOptions *node_options,
          EdgeOptions **edge_options)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        PyErr_Format(PyExc_IOError, "Failed to open %s", filename);
        return;
    }

    write_svg_opening(file, viewbox_ul_x, viewbox_ul_y, viewbox_dr_x,
                      viewbox_dr_y);
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        write_svg_node(file, layout[i], node_options[i]);
    }
    for (Py_ssize_t u = 0; u < node_count; ++u) {
        for (Py_ssize_t j = 0; j < neighbor_count[u]; ++j) {
            Py_ssize_t v = adj_list[u][j];
            if (u > v) {
                continue;
            }
            write_svg_edge(file, layout[u], layout[v], edge_options[u][j]);
        }
    }
    write_svg_closing(file);

    fclose(file);
    return;
}

void
write_svg_opening(FILE *file, uint16_t viewbox_ul_x, uint16_t viewbox_ul_y,
                  uint16_t viewbox_dr_x, uint16_t viewbox_dr_y)
{
    fprintf(file,
            "<svg version=\"1.1\" viewBox=\"%u %u %u %u\" "
            "xmlns=\"http://www.w3.org/2000/svg\">",
            (unsigned int) viewbox_ul_x, (unsigned int) viewbox_ul_y,
            (unsigned int) viewbox_dr_x, (unsigned int) viewbox_dr_y);
}

void
write_svg_closing(FILE *file)
{
    fprintf(file, "</svg>");
}

void
write_svg_node(FILE *file, Point2d point, NodeOptions node_options)
{
    fprintf(file, "<");
    if (strcmp(node_options.shape, "circle") == 0) {
        fprintf(file, "circle cx=\"%u\" cy=\"%u\" r=\"%u\" ",
                (unsigned int) point.x, (unsigned int) point.y,
                (unsigned int) sqrt(node_options.shape_size / M_PI));
    }
    else {
        PyErr_Format(PyExc_ValueError,
                     "Invalid shape argument to write_svg_node: %s",
                     node_options.shape);
        return;
    }

    fprintf(file, "stroke=\"%s\" fill=\"%s\" stroke-width=\"%u\" ",
            node_options.stroke, node_options.fill,
            (unsigned int) node_options.stroke_width);

    fprintf(file, "/>");
}

void
write_svg_edge(FILE *file, Point2d point0, Point2d point1,
               EdgeOptions edge_options)
{
    fprintf(file, "<");

    fprintf(file,
            "line x1=\"%u\" x2=\"%u\" y1=\"%u\" y2=\"%u\" stroke=\"%s\" "
            "stroke-width=\"%u\" ",
            (unsigned int) point0.x, (unsigned int) point1.x,
            (unsigned int) point0.y, (unsigned int) point1.y,
            edge_options.stroke, (unsigned int) edge_options.stroke_width);

    fprintf(file, "/>");
}