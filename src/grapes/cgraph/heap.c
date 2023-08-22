#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "heap.h"

struct MinHeapNode_s {
    Py_ssize_t priority;
    Py_ssize_t key;
};

struct MinHeap_s {
    Py_ssize_t   size;
    Py_ssize_t   max_size;
    MinHeapNode *array;
};

MinHeap *
MinHeap_alloc(Py_ssize_t max_size)
{
    MinHeap *heap = malloc(sizeof(*heap));
    if (heap == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate heap");
        return NULL;
    }

    heap->size = 0;
    heap->max_size = max_size;
    heap->array = malloc(sizeof(*heap->array) * max_size);
    if (heap->array == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate heap array");
        free(heap);
        return NULL;
    }

    return heap;
}

void
MinHeap_free(MinHeap *heap)
{
    if (heap == NULL) {
        return;
    }

    free(heap->array);
    free(heap);
    return;
}

void
MinHeap_insert(MinHeap *heap, Py_ssize_t key, Py_ssize_t priority)
{
    if (heap->size >= heap->max_size) {
        PyErr_Format(PyExc_MemoryError,
                     "Cannot insert key %ld. Heap is already full!", key);
        return;
    }

    Py_ssize_t i = heap->size++;
    heap->array[i].priority = priority;
    heap->array[i].key = key;

    MinHeap_siftdown(heap, 0, heap->size - 1);
    return;
}

Py_ssize_t
MinHeap_extract_min(MinHeap *heap)
{
    if (heap == NULL || heap->size == 0) {
        return -1;
    }

    MinHeapNode lastelt = heap->array[--heap->size];
    Py_ssize_t  min_key = heap->array[0].key;
    heap->array[0] = lastelt;
    MinHeap_siftup(heap, 0);
    return min_key;
}

int
MinHeap_is_empty(MinHeap *heap)
{
    return heap->size == 0;
}

void
MinHeap_siftdown(MinHeap *heap, Py_ssize_t startpos, Py_ssize_t pos)
{
    MinHeapNode newitem = heap->array[pos];
    Py_ssize_t  parentpos;
    MinHeapNode parent;
    while (pos > startpos) {
        parentpos = (pos - 1) >> 1;
        parent = heap->array[parentpos];
        if (newitem.priority < parent.priority) {
            heap->array[pos] = parent;
            pos = parentpos;
            continue;
        }
        break;
    }
    heap->array[pos] = newitem;
    return;
}

void
MinHeap_siftup(MinHeap *heap, Py_ssize_t pos)
{
    Py_ssize_t  endpos = heap->size;
    Py_ssize_t  startpos = pos;
    MinHeapNode newitem = heap->array[pos];
    Py_ssize_t  childpos = 2 * pos + 1;
    Py_ssize_t  rightpos;
    while (childpos < endpos) {
        rightpos = childpos + 1;
        if (rightpos < endpos &&
            heap->array[childpos].priority >= heap->array[rightpos].priority) {
            childpos = rightpos;
        }
        heap->array[pos] = heap->array[childpos];
        pos = childpos;
        childpos = 2 * pos + 1;
    }
    heap->array[pos] = newitem;
    MinHeap_siftdown(heap, startpos, pos);
    return;
}

void
MinHeap_print(MinHeap *heap)
{
    printf("heap size=%ld values=", heap->size);
    for (Py_ssize_t i = 0; i < heap->size; ++i) {
        printf("%ld, ", heap->array[i].key);
    }
    printf("\n");
}
