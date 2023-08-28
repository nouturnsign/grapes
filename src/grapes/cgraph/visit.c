#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "deque.h"
#include "heap.h"
#include "macros.h"

void
visit_dijkstra(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
               Py_ssize_t node_end, Py_ssize_t directed_edge_count,
               Py_ssize_t *srcs, Py_ssize_t src_count, double **weight,
               double *dist, Py_ssize_t *prev)
{
    int     *visited = NULL;
    MinHeap *heap = NULL;

    visited = malloc(sizeof(*visited) * node_end);
    if (visited == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate visited");
        goto err;
    }

    heap = MinHeap_alloc(directed_edge_count, src_count);
    if (heap == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate heap");
        goto err;
    }

    for (Py_ssize_t i = 0; i < node_end; ++i) {
        dist[i] = INFINITY;
        visited[i] = GRAPES_FALSE;
        if (neighbor_count[i] == GRAPES_NO_NODE) {
            prev[i] = GRAPES_NO_NODE;
        }
        else {
            prev[i] = node_end;
        }
    }

    for (Py_ssize_t i = 0; i < src_count; ++i) {
        Py_ssize_t src = srcs[i];
        dist[src] = 0;
        prev[src] = src;
        if (MinHeap_insert(heap, src, 0.0) == -1) {
            PyErr_Format(PyExc_ValueError,
                         "Heap is full or cannot allocate more memory");
            goto err;
        }
    }

    while (!MinHeap_is_empty(heap)) {
        Py_ssize_t u = MinHeap_extract_min(heap);
        if (u == -1) {
            PyErr_Format(PyExc_ValueError, "Heap is empty");
            goto err;
        }
        if (visited[u]) {
            continue;
        }
        visited[u] = GRAPES_TRUE;
        for (Py_ssize_t j = 0; j < neighbor_count[u]; ++j) {
            Py_ssize_t v = adj_list[u][j];
            double     w = weight[u][j];
            if (visited[v]) {
                continue;
            }
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                if (MinHeap_insert(heap, v, dist[v]) == -1) {
                    PyErr_Format(
                        PyExc_ValueError,
                        "Heap is full or cannot allocate more memory");
                    goto err;
                }
            }
        }
    }

err:
    free(visited);
    MinHeap_free(heap);
    return;
}

int
visit_bellman_ford(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                   Py_ssize_t node_end, Py_ssize_t *srcs, Py_ssize_t src_count,
                   double **weight, double *dist, Py_ssize_t *prev)
{
    // SPFA variant + SLF technique
    // https://en.wikipedia.org/wiki/Shortest_path_faster_algorithm#Optimization_techniques
    int         retvalue = -1;
    Deque      *deque = NULL;
    short      *in_deque = NULL;
    Py_ssize_t *count = NULL;

    deque = Deque_alloc();
    if (deque == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate deque");
        goto err;
    }

    in_deque = malloc(sizeof(*in_deque) * node_end);
    if (in_deque == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate in_deque");
        goto err;
    }

    count = malloc(sizeof(*count) * node_end);
    if (count == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate count");
        goto err;
    }

    for (Py_ssize_t u = 0; u < node_end; ++u) {
        dist[u] = INFINITY;
        prev[u] = node_end;
        in_deque[u] = GRAPES_FALSE;
        count[u] = 0;
    }

    for (Py_ssize_t i = 0; i < src_count; ++i) {
        Py_ssize_t src = srcs[i];
        dist[src] = 0;
        prev[src] = src;
        Deque_push_back(deque, src);
        if (PyErr_Occurred()) {
            goto err;
        }
        in_deque[src] = GRAPES_TRUE;
    }

    while (!Deque_is_empty(deque)) {
        Py_ssize_t u = Deque_pop_front(deque);
        in_deque[u] = GRAPES_FALSE;
        for (Py_ssize_t j = 0; j < neighbor_count[u]; ++j) {
            Py_ssize_t v = adj_list[u][j];
            double     w = weight[u][j];
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                if (in_deque[v]) {
                    continue;
                }

                if (++count[v] == node_end) {
                    retvalue = 1;
                    goto err;
                }
                if (Deque_is_empty(deque) ||
                    dist[v] >= dist[Deque_peek_front(deque)]) {
                    Deque_push_back(deque, v);
                }
                else {
                    Deque_push_front(deque, v);
                }
                if (PyErr_Occurred()) {
                    goto err;
                }
                in_deque[v] = GRAPES_TRUE;
            }
        }
    }

    retvalue = 0;
err:
    free(in_deque);
    free(count);
    Deque_free(deque);
    return retvalue;
}

int
visit_floyd_warshall(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                     Py_ssize_t node_end, double **weight, double **dist,
                     Py_ssize_t **prev)
{
    int retvalue = -1;

    for (Py_ssize_t u = 0; u < node_end; ++u) {
        dist[u][u] = 0;
        prev[u][u] = u;
        for (Py_ssize_t j = 0; j < neighbor_count[u]; ++j) {
            Py_ssize_t v = adj_list[u][j];
            dist[u][v] = weight[u][j];
            prev[u][v] = u;
        }
    }

    for (Py_ssize_t k = 0; k < node_end; ++k) {
        for (Py_ssize_t i = 0; i < node_end; ++i) {
            for (Py_ssize_t j = 0; j < node_end; ++j) {
                if (dist[i][j] > dist[i][k] + dist[k][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    prev[i][j] = prev[k][j];
                }
            }
        }
    }

    for (Py_ssize_t i = 0; i < node_end; ++i) {
        if (dist[i][i] < 0) {
            retvalue = 1;
            goto err;
        }
    }

    retvalue = 0;
err:
    return retvalue;
}

Py_ssize_t
visit_component(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
                Py_ssize_t src, short *visited)
{
    visited[src] = GRAPES_TRUE;
    Py_ssize_t size = 1;
    Deque     *queue = Deque_alloc();  // push_back, pop_front
    if (PyErr_Occurred()) {
        return -1;
    }
    Deque_push_back(queue, src);
    if (PyErr_Occurred()) {
        Deque_free(queue);
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
                    Deque_free(queue);
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
        Deque_free(queue);
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
