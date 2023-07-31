#ifndef GRAPES_GRAPES_CGRAPH_DEQUE_H_
#define GRAPES_GRAPES_CGRAPH_DEQUE_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct DequeNode DequeNode;
typedef struct Deque     Deque;

Deque*     Deque_alloc();
void       Deque_free(Deque* deque);
void       Deque_push_front(Deque* deque, Py_ssize_t value);
Py_ssize_t Deque_pop_front(Deque* deque);
void       Deque_push_back(Deque* deque, Py_ssize_t value);
Py_ssize_t Deque_pop_back(Deque* deque);
short      Deque_is_empty(Deque* deque);
void       Deque_print(Deque* deque);

#endif // GRAPES_GRAPES_CGRAPH_DEQUE_H_