#ifndef GRAPES_GRAPES_CGRAPH_TRAV_H_
#define GRAPES_GRAPES_CGRAPH_TRAV_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

Py_ssize_t visit(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                 Py_ssize_t src, short *visited);
short      visit_color(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                       Py_ssize_t src, short *color);

#endif  // GRAPES_GRAPES_CGRAPH_TRAV_H_
