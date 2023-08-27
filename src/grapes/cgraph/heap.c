#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "heap.h"

// adapted from: https://dl.acm.org/doi/10.1145/351827.384249
// less naive binary heap

MinHeap *
MinHeap_alloc(Py_ssize_t max_capacity, Py_ssize_t init_capacity)
{
    MinHeap *heap = malloc(sizeof(*heap));
    if (heap == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate heap");
        return NULL;
    }

    heap->size = 0;
    heap->capacity = init_capacity;
    heap->max_capacity = max_capacity;
    heap->data = malloc(sizeof(*heap->data) * (init_capacity + 2));
    if (heap->data == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate heap array");
        free(heap);
        return NULL;
    }

    heap->data[0].key = -1;
    heap->data[init_capacity + 1].key = max_capacity;
    for (Py_ssize_t i = 1; i <= init_capacity; ++i) {
        heap->data[i].key = max_capacity;
    }

    return heap;
}

void
MinHeap_free(MinHeap *heap)
{
    if (heap == NULL) {
        return;
    }

    free(heap->data);
    free(heap);
    return;
}

int
MinHeap_insert(MinHeap *heap, Py_ssize_t key, double priority)
{
    if (heap->size >= heap->capacity) {
        if (heap->capacity > heap->max_capacity) {
            return -1;
        }

        heap->capacity =
            (heap->capacity + (heap->capacity >> 3) + 6) & ~(Py_ssize_t) 3;
        if (heap->capacity > heap->max_capacity) {
            heap->capacity = heap->max_capacity;
        }
        heap->data =
            realloc(heap->data, sizeof(*heap->data) * (heap->capacity + 2));
        if (heap->data == NULL) {
            return -1;
        }
        for (Py_ssize_t i = heap->size + 1; i <= heap->capacity + 1; ++i) {
            heap->data[i].key = heap->max_capacity;
        }
    }

    MinHeapNode *dat = heap->data;
    Py_ssize_t   hole = ++heap->size;
    Py_ssize_t   pred = hole >> 1;
    Py_ssize_t   pred_key = dat[pred].key;
    while (pred_key > key) {
        dat[hole].key = pred_key;
        dat[hole].priority = dat[pred].priority;
        hole = pred;
        pred >>= 1;
        pred_key = dat[pred].key;
    }

    dat[hole].key = key;
    dat[hole].priority = priority;

    return 0;
}

Py_ssize_t
MinHeap_extract_min(MinHeap *heap)
{
    if (heap->size <= 0) {
        return -1;
    }

    MinHeapNode *dat = heap->data;

    Py_ssize_t min_key = dat[1].key;

    Py_ssize_t hole = 1;
    Py_ssize_t succ = 2;
    Py_ssize_t sz = heap->size;
    while (succ < sz) {
        Py_ssize_t key1 = dat[succ].key;
        Py_ssize_t key2 = dat[succ + 1].key;
        if (key1 > key2) {
            ++succ;
            dat[hole].key = key2;
            dat[hole].priority = dat[succ].priority;
        }
        else {
            dat[hole].key = key1;
            dat[hole].priority = dat[succ].priority;
        }
        hole = succ;
        succ <<= 1;
    }

    Py_ssize_t bubble = dat[sz].key;
    Py_ssize_t pred = hole >> 1;
    while (dat[pred].key > bubble) {
        dat[hole] = dat[pred];
        hole = pred;
        pred >>= 1;
    }

    dat[hole].key = bubble;
    dat[hole].priority = dat[sz].priority;

    dat[heap->size--].key = heap->max_capacity;

    return min_key;
}

int
MinHeap_is_empty(MinHeap *heap)
{
    return (heap->size <= 0);
}