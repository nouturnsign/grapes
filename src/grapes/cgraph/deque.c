#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "deque.h"

typedef struct DequeNode
{
    Py_ssize_t value;
    DequeNode* prev;
    DequeNode* next;
} DequeNode;

typedef struct Deque
{
    DequeNode* head;
    DequeNode* tail;
} Deque;

Deque* Deque_alloc()
{
    Deque* deque = malloc(sizeof(*deque));
    if (deque == NULL)
    {
        fprintf(stderr, "Failed to allocate deque\n");
        return NULL;
    }

    deque->head = NULL;
    deque->tail = NULL;
    return deque;
}

void Deque_free(Deque* deque)
{
    DequeNode* curr = deque->head;
    DequeNode* next;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(deque);
    deque = NULL;
}

void Deque_push_front(Deque* deque, Py_ssize_t value)
{
    DequeNode* curr = malloc(sizeof(*curr));
    if (curr == NULL)
    {
        fprintf(stderr, "Failed to allocate curr node\n");
        return;
    }

    curr->prev = NULL;
    curr->value = value;
    DequeNode* head = deque->head;
    if (head == NULL)
    {
        deque->tail = curr;
    }
    else
    {
        head->prev = curr;
    }

    curr->next = head;
    deque->head = curr;
}

Py_ssize_t Deque_pop_front(Deque* deque)
{
    Py_ssize_t value = deque->head->value;
    deque->head = deque->head->next;
    if (deque->head == NULL)
    {
        deque->tail = NULL;
    }
    return value;
}

void Deque_push_back(Deque* deque, Py_ssize_t value)
{
    DequeNode* curr = malloc(sizeof(*curr));
    if (curr == NULL)
    {
        fprintf(stderr, "Failed to allocate curr node\n");
        return;
    }

    curr->next = NULL;
    curr->value = value;
    DequeNode* tail = deque->tail;
    if (tail == NULL)
    {
        deque->head = curr;
    }
    else
    {
        tail->next = curr;
    }

    curr->prev = tail;
    deque->tail = curr;
}

Py_ssize_t Deque_pop_back(Deque* deque)
{
    Py_ssize_t value = deque->tail->value;
    deque->tail = deque->tail->prev;
    if (deque->tail == NULL)
    {
        deque->head = NULL;
    }
    return value;
}

short Deque_is_empty(Deque* deque)
{
    return (deque->head == NULL && deque->tail == NULL);
}

void Deque_print(Deque* deque)
{
    printf("deque values=");
    DequeNode* curr = deque->head;
    while (curr != NULL)
    {
        printf("%ld, ", curr->value);
        curr = curr->next;
    }
    printf("\n");
}
