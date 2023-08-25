#ifndef GRAPES_GRAPES_CGRAPH_TRAV_H_
#define GRAPES_GRAPES_CGRAPH_TRAV_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

void visit_dijkstra(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                    Py_ssize_t node_count, Py_ssize_t directed_edge_count,
                    Py_ssize_t *srcs, Py_ssize_t src_count, double **weight,
                    double *dist, Py_ssize_t *prev);
int  visit_bellman_ford(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                        Py_ssize_t node_count, Py_ssize_t *srcs,
                        Py_ssize_t src_count, double **weight, double *dist,
                        Py_ssize_t *prev);
int  visit_floyd_warshall(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                          Py_ssize_t node_count, double **weight, double **dist,
                          Py_ssize_t **prev);
Py_ssize_t visit_component(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                           Py_ssize_t src, short *visited);
short      visit_color(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                       Py_ssize_t src, short *color);

#endif  // GRAPES_GRAPES_CGRAPH_TRAV_H_
