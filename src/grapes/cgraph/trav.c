#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "deque.h"
#include "macros.h"

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
