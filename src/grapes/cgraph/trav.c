#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "deque.h"
#include "heap.h"
#include "macros.h"

void
visit_dijkstra(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
               Py_ssize_t node_count, Py_ssize_t src, double **weight,
               double *dist, Py_ssize_t *prev)
{
    short *visited = malloc(sizeof(*visited) * node_count);
    if (visited == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate visited");
        return;
    }

    for (Py_ssize_t i = 0; i < node_count; ++i) {
        dist[i] = INFINITY;
        visited[i] = GRAPES_FALSE;
        prev[i] = node_count;
    }
    dist[src] = 0;
    visited[src] = GRAPES_TRUE;
    prev[src] = src;

    MinHeap *heap = MinHeap_alloc((node_count * (node_count - 1)) / 2);
    if (PyErr_Occurred()) {
        free(visited);
        return;
    }
    MinHeap_insert(heap, src, 0);
    if (PyErr_Occurred()) {
        free(visited);
        return;
    }
    Py_ssize_t u, v;
    double     w;
    while (!MinHeap_is_empty(heap)) {
        u = MinHeap_extract_min(heap);
        visited[u] = GRAPES_TRUE;
        for (Py_ssize_t j = 0; j < neighbor_count[u]; ++j) {
            v = adj_list[u][j];
            if (visited[v]) {
                continue;
            }
            w = weight[u][j];

            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                MinHeap_insert(heap, v, dist[v]);
                if (PyErr_Occurred()) {
                    free(visited);
                    MinHeap_free(heap);
                    return;
                }
            }
        }
    }

    free(visited);
    MinHeap_free(heap);
}

Py_ssize_t
visit(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count, Py_ssize_t src,
      short *visited)
{
    visited[src] = GRAPES_TRUE;
    Py_ssize_t size = 1;
    Deque     *queue = Deque_alloc();  // push_back, pop_front
    if (PyErr_Occurred()) {
        return -1;
    }
    Deque_push_back(queue, src);
    if (PyErr_Occurred()) {
        return -1;
    }
    while (!Deque_is_empty(queue)) {
        Py_ssize_t curr = Deque_pop_front(queue);
        for (Py_ssize_t j = 0; j < neighbor_count[curr]; ++j) {
            Py_ssize_t neighbor = adj_list[curr][j];
            if (!visited[neighbor]) {
                visited[neighbor] = GRAPES_TRUE;
                ++size;
                Deque_push_back(queue, neighbor);
                if (PyErr_Occurred()) {
                    return -1;
                }
            }
        }
    }
    Deque_free(queue);
    return size;
}

short
visit_color(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count, Py_ssize_t src,
            short *color)
{
    if (color[src] != GRAPES_NO_COLOR) {
        return GRAPES_TRUE;
    }
    color[src] = GRAPES_RED;
    Deque *queue = Deque_alloc();  // push_back, pop_front
    if (PyErr_Occurred()) {
        return -1;
    }
    Deque_push_back(queue, src);
    if (PyErr_Occurred()) {
        return -1;
    }
    while (!Deque_is_empty(queue)) {
        Py_ssize_t curr = Deque_pop_front(queue);
        for (Py_ssize_t j = 0; j < neighbor_count[curr]; ++j) {
            Py_ssize_t neighbor = adj_list[curr][j];
            if (color[neighbor] == GRAPES_NO_COLOR) {
                color[neighbor] =
                    (color[curr] == GRAPES_RED) ? GRAPES_BLUE : GRAPES_RED;
                Deque_push_back(queue, neighbor);
                if (PyErr_Occurred()) {
                    Deque_free(queue);
                    return -1;
                }
            }
            else if (color[neighbor] == color[curr]) {
                Deque_free(queue);
                return GRAPES_FALSE;
            }
        }
    }
    Deque_free(queue);
    return GRAPES_TRUE;
}
