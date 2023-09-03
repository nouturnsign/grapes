#ifndef GRAPES_GRAPES_CGRAPH_CENTRALITY_H_
#define GRAPES_GRAPES_CGRAPH_CENTRALITY_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

Py_ssize_t centrality_outdegree(Py_ssize_t *neighbor_count, Py_ssize_t u);

#endif  // GRAPES_GRAPES_CGRAPH_CENTRALITY_H_