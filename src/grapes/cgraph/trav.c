#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "deque.h"
#include "heap.h"
#include "macros.h"

double
get_weight(PyObject *weight, Py_ssize_t u, Py_ssize_t v)
{
    const int failed = -1;
    double    w;
    PyObject *uvargs;
    PyObject *ret_value;

    uvargs = Py_BuildValue("(nn)", u, v);
    if (uvargs == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "Unable to format args given u=%ld and v=%ld", u, v);
        return failed;
    }
    ret_value = PyObject_Call(weight, uvargs, NULL);
    if (ret_value == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "Unable to call weight function on args given "
                     "weight=%R and uvargs=%R",
                     weight, uvargs);
        return failed;
    }
    w = PyFloat_AsDouble(ret_value);
    if (w == -1 && PyErr_Occurred() != NULL) {
        PyErr_Format(PyExc_ValueError,
                     "weight function returned a non-float value "
                     "given ret_value=%R",
                     ret_value);
        return failed;
    }

    return w;
}

void
visit_dijkstra(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count,
               Py_ssize_t node_count, Py_ssize_t src, PyObject *weight,
               Py_ssize_t *dist, Py_ssize_t *prev)
{
    short *visited = malloc(sizeof(*visited) * node_count);
    if (visited == NULL) {
        fprintf(stderr, "Failed to allocate visited\n");
        return;
    }

    for (Py_ssize_t i = 0; i < node_count; ++i) {
        dist[i] = PY_SSIZE_T_MAX;
        visited[i] = GRAPES_FALSE;
        prev[i] = node_count;
    }
    dist[src] = 0;
    visited[src] = GRAPES_TRUE;
    prev[src] = src;

    MinHeap *heap = MinHeap_alloc((node_count * (node_count - 1)) / 2);
    MinHeap_insert(heap, src, 0);
    Py_ssize_t u, v;
    double     w;
    while (!MinHeap_is_empty(heap)) {
        u = MinHeap_extract_min(heap);
        visited[u] = GRAPES_TRUE;
        for (Py_ssize_t i = 0; i < neighbor_count[u]; ++i) {
            v = adj_list[u][i];
            if (visited[v]) {
                continue;
            }
            w = get_weight(weight, u, v);
            if (w == -1 && PyErr_Occurred() != NULL) {
                return;
            }

            if (dist[v] - dist[u] > w) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                MinHeap_insert(heap, v, dist[v]);
            }
        }
    }

    free(visited);
    visited = NULL;
    MinHeap_free(heap);
    heap = NULL;
}

Py_ssize_t
visit(Py_ssize_t **adj_list, Py_ssize_t *neighbor_count, Py_ssize_t src,
      short *visited)
{
    visited[src] = GRAPES_TRUE;
    Py_ssize_t size = 1;
    Deque     *queue = Deque_alloc();  // push_back, pop_front
    Deque_push_back(queue, src);
    while (!Deque_is_empty(queue)) {
        Py_ssize_t curr = Deque_pop_front(queue);
        for (Py_ssize_t j = 0; j < neighbor_count[curr]; ++j) {
            Py_ssize_t neighbor = adj_list[curr][j];
            if (!visited[neighbor]) {
                visited[neighbor] = GRAPES_TRUE;
                ++size;
                Deque_push_back(queue, neighbor);
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
    Deque_push_back(queue, src);
    while (!Deque_is_empty(queue)) {
        Py_ssize_t curr = Deque_pop_front(queue);
        for (Py_ssize_t j = 0; j < neighbor_count[curr]; ++j) {
            Py_ssize_t neighbor = adj_list[curr][j];
            if (color[neighbor] == GRAPES_NO_COLOR) {
                color[neighbor] =
                    (color[curr] == GRAPES_RED) ? GRAPES_BLUE : GRAPES_RED;
                Deque_push_back(queue, neighbor);
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
