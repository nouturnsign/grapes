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
    char          *shape;
    char          *stroke;
    char          *fill;
    unsigned short stroke_width;
    char          *label;
    // custom
    double shape_size;
} NodeOptions;

typedef struct EdgeOptions {
    // svg
    char          *stroke;
    unsigned short stroke_width;
    char          *label;
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
        node_options[i].label = "";
        node_options[i].shape_size = 300;
    }

    return node_options;
}

void
Options_update(NodeOptions *node_options, EdgeOptions **edge_options,
               Py_ssize_t *neighbor_count, Py_ssize_t node_count,
               PyObject *options)
{
    PyObject *node_dict = PyDict_GetItemString(options, "node");
    if (node_dict != NULL) {
        PyObject *shape = PyDict_GetItemString(node_dict, "shape");
        if (shape != NULL) {
            for (Py_ssize_t i = 0; i < node_count; ++i) {
                PyObject *value;
                if (PyObject_IsInstance(shape, (PyObject *) &PyList_Type)) {
                    value = PyList_GetItem(shape, i);
                }
                else {
                    value = shape;
                }
                node_options[i].shape = PyUnicode_AsUTF8(value);
            }
        }
        PyObject *stroke = PyDict_GetItemString(node_dict, "stroke");
        if (stroke != NULL) {
            for (Py_ssize_t i = 0; i < node_count; ++i) {
                PyObject *value;
                if (PyObject_IsInstance(stroke, (PyObject *) &PyList_Type)) {
                    value = PyList_GetItem(stroke, i);
                }
                else {
                    value = stroke;
                }
                node_options[i].stroke = PyUnicode_AsUTF8(value);
            }
        }
        PyObject *fill = PyDict_GetItemString(node_dict, "fill");
        if (fill != NULL) {
            for (Py_ssize_t i = 0; i < node_count; ++i) {
                PyObject *value;
                if (PyObject_IsInstance(fill, (PyObject *) &PyList_Type)) {
                    value = PyList_GetItem(fill, i);
                }
                else {
                    value = fill;
                }
                node_options[i].fill = PyUnicode_AsUTF8(value);
            }
        }
        PyObject *stroke_width =
            PyDict_GetItemString(node_dict, "stroke_width");
        if (stroke_width != NULL) {
            for (Py_ssize_t i = 0; i < node_count; ++i) {
                PyObject *value;
                if (PyObject_IsInstance(stroke_width,
                                        (PyObject *) &PyList_Type)) {
                    value = PyList_GetItem(stroke_width, i);
                }
                else {
                    value = stroke_width;
                }
                node_options[i].stroke_width =
                    (unsigned short) PyLong_AsUnsignedLong(value);
            }
        }
        PyObject *label = PyDict_GetItemString(node_dict, "label");
        if (label != NULL) {
            for (Py_ssize_t i = 0; i < node_count; ++i) {
                PyObject *value;
                if (PyObject_IsInstance(label, (PyObject *) &PyList_Type)) {
                    value = PyList_GetItem(label, i);
                }
                else {
                    value = label;
                }
                node_options[i].label = PyUnicode_AsUTF8(value);
            }
        }
        PyObject *shape_size = PyDict_GetItemString(node_dict, "shape_size");
        if (shape_size != NULL) {
            for (Py_ssize_t i = 0; i < node_count; ++i) {
                PyObject *value;
                if (PyObject_IsInstance(shape_size,
                                        (PyObject *) &PyList_Type)) {
                    value = PyList_GetItem(shape_size, i);
                }
                else {
                    value = shape_size;
                }
                node_options[i].shape_size = PyFloat_AsDouble(value);
            }
        }
    }

    // TODO: use edge_options
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
            free(edge_options);
            return NULL;
        }
    }

    for (Py_ssize_t i = 0; i < node_count; ++i) {
        for (Py_ssize_t j = 0; j < neighbor_count[i]; ++j) {
            edge_options[i][j].stroke = "black";
            edge_options[i][j].stroke_width = 2;
            edge_options[i][j].label = "";
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
          int viewbox_ul_x, int viewbox_ul_y, int viewbox_dr_x,
          int viewbox_dr_y, NodeOptions *node_options,
          EdgeOptions **edge_options)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        PyErr_Format(PyExc_IOError, "Failed to open %s", filename);
        return;
    }

    write_svg_opening(file, viewbox_ul_x, viewbox_ul_y, viewbox_dr_x,
                      viewbox_dr_y);
    for (Py_ssize_t u = 0; u < node_count; ++u) {
        for (Py_ssize_t j = 0; j < neighbor_count[u]; ++j) {
            Py_ssize_t v = adj_list[u][j];
            if (u > v) {
                continue;
            }
            write_svg_edge(file, layout[u], layout[v], edge_options[u][j]);
        }
    }
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        write_svg_node(file, layout[i], node_options[i]);
    }
    write_svg_closing(file);

    fclose(file);
    return;
}

void
write_svg_opening(FILE *file, int viewbox_ul_x, int viewbox_ul_y,
                  int viewbox_dr_x, int viewbox_dr_y)
{
    fprintf(file,
            "<svg version=\"1.1\" viewBox=\"%d %d %d %d\" "
            "xmlns=\"http://www.w3.org/2000/svg\">",
            viewbox_ul_x, viewbox_ul_y, viewbox_dr_x, viewbox_dr_y);
}

void
write_svg_closing(FILE *file)
{
    fprintf(file, "</svg>");
}

void
write_svg_node(FILE *file, Point2d point, NodeOptions node_options)
{
    int cx, cy;
    fprintf(file, "<");
    if (strcmp(node_options.shape, "circle") == 0) {
        cx = (int) point.x;
        cy = (int) point.y;
        fprintf(file, "circle cx=\"%d\" cy=\"%d\" r=\"%d\" ", cx, cy,
                (int) sqrt(node_options.shape_size / M_PI));
    }
    else {
        PyErr_Format(PyExc_ValueError,
                     "Invalid shape argument to write_svg_node: %s",
                     node_options.shape);
        return;
    }

    fprintf(file, "stroke=\"%s\" fill=\"%s\" stroke-width=\"%hu\" ",
            node_options.stroke, node_options.fill, node_options.stroke_width);

    fprintf(file, "/>");

    if (strcmp(node_options.label, "") != 0) {
        fprintf(file,
                "<text x=\"%d\" y=\"%d\" dominant-baseline=\"central\" "
                "text-anchor=\"middle\" >%s</text>",
                cx, cy, node_options.label);
    }
}

void
write_svg_edge(FILE *file, Point2d point0, Point2d point1,
               EdgeOptions edge_options)
{
    fprintf(file, "<");

    fprintf(file,
            "line x1=\"%d\" x2=\"%d\" y1=\"%d\" y2=\"%d\" stroke=\"%s\" "
            "stroke-width=\"%hu\" ",
            (int) point0.x, (int) point1.x, (int) point0.y, (int) point1.y,
            edge_options.stroke, edge_options.stroke_width);

    fprintf(file, "/>");

    // TODO: use edge_options
}
