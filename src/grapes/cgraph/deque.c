#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "deque.h"

#include "macros.h"

struct DequeNode_s {
    Py_ssize_t value;
    DequeNode *prev;
    DequeNode *next;
};

struct Deque_s {
    DequeNode *head;
    DequeNode *tail;
};

Deque *
Deque_alloc(void)
{
    Deque *deque = malloc(sizeof(*deque));
    if (deque == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate deque");
        return NULL;
    }

    deque->head = NULL;
    deque->tail = NULL;
    return deque;
}

void
Deque_free(Deque *deque)
{
    if (deque == NULL) {
        return;
    }

    DequeNode *curr = deque->head;
    DequeNode *next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(deque);
}

void
Deque_push_front(Deque *deque, Py_ssize_t value)
{
    DequeNode *curr = malloc(sizeof(*curr));
    if (curr == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate curr node");
        return;
    }

    curr->prev = NULL;
    curr->value = value;
    DequeNode *head = deque->head;
    if (head == NULL) {
        deque->tail = curr;
    }
    else {
        head->prev = curr;
    }

    curr->next = head;
    deque->head = curr;
}

Py_ssize_t
Deque_pop_front(Deque *deque)
{
    Py_ssize_t value = deque->head->value;
    DequeNode *next = deque->head->next;
    free(deque->head);
    deque->head = next;
    if (deque->head == NULL) {
        deque->tail = NULL;
    }
    return value;
}

Py_ssize_t
Deque_peek_front(Deque *deque)
{
    return deque->head->value;
}

void
Deque_push_back(Deque *deque, Py_ssize_t value)
{
    DequeNode *curr = malloc(sizeof(*curr));
    if (curr == NULL) {
        PyErr_Format(PyExc_MemoryError, "Failed to allocate curr node");
        return;
    }

    curr->next = NULL;
    curr->value = value;
    DequeNode *tail = deque->tail;
    if (tail == NULL) {
        deque->head = curr;
    }
    else {
        tail->next = curr;
    }

    curr->prev = tail;
    deque->tail = curr;
}

Py_ssize_t
Deque_pop_back(Deque *deque)
{
    Py_ssize_t value = deque->tail->value;
    DequeNode *prev = deque->tail->prev;
    free(deque->tail);
    deque->tail = prev;
    if (deque->tail == NULL) {
        deque->head = NULL;
    }
    return value;
}

Py_ssize_t
Deque_peek_back(Deque *deque)
{
    return deque->tail->value;
}

short
Deque_contains(Deque *deque, Py_ssize_t value)
{
    DequeNode *curr = deque->head;
    while (curr != NULL) {
        if (curr->value == value) {
            return GRAPES_TRUE;
        }
        curr = curr->next;
    }
    return GRAPES_FALSE;
}

short
Deque_is_empty(Deque *deque)
{
    return (deque->head == NULL && deque->tail == NULL);
}
