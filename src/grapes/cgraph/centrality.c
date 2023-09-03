#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "centrality.h"

Py_ssize_t
centrality_outdegree(Py_ssize_t *neighbor_count, Py_ssize_t u)
{
    return neighbor_count[u];
}