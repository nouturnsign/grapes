#ifndef GRAPES_GRAPES_CGRAPH_HEAP_H_
#define GRAPES_GRAPES_CGRAPH_HEAP_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
    double     priority;
    Py_ssize_t key;
} MinHeapNode;
typedef struct {
    Py_ssize_t   size;
    Py_ssize_t   capacity;
    Py_ssize_t   max_capacity;
    MinHeapNode *data;
} MinHeap;

MinHeap   *MinHeap_alloc(Py_ssize_t max_capacity, Py_ssize_t init_capacity);
void       MinHeap_free(MinHeap *heap);
int        MinHeap_insert(MinHeap *heap, Py_ssize_t key, double priority);
Py_ssize_t MinHeap_extract_min(MinHeap *heap);
int        MinHeap_is_empty(MinHeap *heap);

#endif  // GRAPES_GRAPES_CGRAPH_HEAP_H_