#ifndef GRAPES_GRAPES_CGRAPH_VIS_H_
#define GRAPES_GRAPES_CGRAPH_VIS_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>

#include "cgraph.h"

typedef struct Point2d     Point2d;
typedef struct NodeOptions NodeOptions;
typedef struct EdgeOptions EdgeOptions;

Point2d      *Layout_alloc(Py_ssize_t node_count);
NodeOptions  *NodeOptions_alloc(Py_ssize_t node_count);
EdgeOptions **EdgeOptions_alloc(Py_ssize_t *neighbor_count,
                                Py_ssize_t  node_count);
void          Vis_free(Point2d *layout, NodeOptions *node_options,
                       EdgeOptions **edge_options, Py_ssize_t node_count);
void layout_circular(Point2d *layout, Py_ssize_t begin, Py_ssize_t end,
                     double radius, double theta0, double cx, double cy);
void write_svg(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
               Py_ssize_t node_count, char *filename, Point2d *layout,
               uint16_t width, uint16_t height, NodeOptions *node_options,
               EdgeOptions **edge_options);
void write_svg_opening(FILE *file, uint16_t width, uint16_t height);
void write_svg_closing(FILE *file);
void write_svg_node(FILE *file, Point2d point, NodeOptions node_options);
void write_svg_edge(FILE *file, Point2d point0, Point2d point1,
                    EdgeOptions edge_options);

#endif  // GRAPES_GRAPES_CGRAPH_VIS_H_