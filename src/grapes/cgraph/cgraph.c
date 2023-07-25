#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cgraph.h"

#include "heap.h"

PyMODINIT_FUNC PyInit_cgraph(void)
{
    PyObject* m;
    if (PyType_Ready(&GraphType) < 0)
    {
        return NULL;
    }

    m = PyModule_Create(&cgraphmodule);
    if (m == NULL)
    {
        return NULL;
    }

    Py_INCREF(&GraphType);
    if (PyModule_AddObject(m, "Graph", (PyObject*) &GraphType) < 0)
    {
        Py_DECREF(&GraphType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

static struct PyModuleDef cgraphmodule = {
    // clang-format off
    PyModuleDef_HEAD_INIT,
    .m_name = "cgraph",
    .m_doc = PyDoc_STR("Grapes core functionality written in C"),
    .m_size = -1,
}; // clang-format on

typedef struct GraphObject
{ // clang-format off
    PyObject_HEAD
    Py_ssize_t** adj_list; // list of adjacency lists (adj_list[i] = array of neighbors to node i)
    Py_ssize_t node_count;
    Py_ssize_t max_node_count; // current maximum number of nodes allocated
    Py_ssize_t* neighbor_count;
    Py_ssize_t* max_neighbor_count; // current maximum number of neighbors (max_neighbor_count[i] = current maximum number of neighbors allocated to node i)
} GraphObject; // clang-format on

// clang-format off
static PyTypeObject GraphType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "grapes.cgraph.Graph",
    .tp_doc = PyDoc_STR("Undirected graph object."),
    .tp_basicsize = sizeof(GraphObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_dealloc = (destructor) Graph_dealloc,
    .tp_new = Graph_new,
    .tp_init = (initproc) Graph_init,
    .tp_methods = Graph_methods,
};
// clang-format on

static void Graph_dealloc(GraphObject* self)
{
    for (Py_ssize_t i = 0; i < self->max_node_count; ++i)
    {
        free(self->adj_list[i]);
        self->adj_list[i] = NULL;
    }
    free(self->adj_list);
    self->adj_list = NULL;
    free(self->neighbor_count);
    self->neighbor_count = NULL;
    free(self->max_neighbor_count);
    self->max_neighbor_count = NULL;
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* Graph_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    GraphObject* self;
    self = (GraphObject*) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->adj_list = NULL;
        self->node_count = 0;
        self->max_node_count = 0;
        self->neighbor_count = NULL;
        self->max_neighbor_count = NULL;
    }
    return (PyObject*) self;
}

static int Graph_init(GraphObject* self, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = {"node_count", NULL};
    Py_ssize_t   node_count;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "n", kwlist, &node_count))
    {
        return -1;
    }

    if (node_count < 0)
    {
        PyErr_Format(PyExc_ValueError,
                     "node_count should be nonnegative, but given %ld",
                     node_count);
        return -1;
    }

    self->adj_list = malloc(sizeof(*self->adj_list) * node_count);
    if (self->adj_list == NULL)
    {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc adj_list at memory address %p",
                     (void*) self->adj_list);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i)
    {
        self->adj_list[i] = NULL;
    }

    self->node_count = node_count;
    self->max_node_count = node_count;

    self->neighbor_count = malloc(sizeof(*self->neighbor_count) * node_count);
    if (self->neighbor_count == NULL)
    {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc neighbor_count at memory address %p",
                     (void*) self->neighbor_count);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i)
    {
        self->neighbor_count[i] = 0;
    }

    self->max_neighbor_count =
        malloc(sizeof(*self->max_neighbor_count) * node_count);
    if (self->max_neighbor_count == NULL)
    {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc max_neighbor_count at memory address %p",
                     (void*) self->max_neighbor_count);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i)
    {
        self->max_neighbor_count[i] = 0;
    }
    return 0;
}

// clang-format off
static PyMethodDef Graph_methods[] = {
    {"get_node_count", (PyCFunction) Graph_get_node_count, METH_NOARGS, 
     "Return the number of nodes in the graph."
    },
    {"add_node", (PyCFunction) Graph_add_node, METH_NOARGS, 
     "Add a node to the graph, returning the newest node."},
    {"add_edge", (PyCFunction) Graph_add_edge, METH_VARARGS | METH_KEYWORDS,
     "Add an undirected edge to the graph given existing nodes."},
    {"dijkstra_path", (PyCFunction) Graph_dijkstra_path, METH_VARARGS | METH_KEYWORDS,
     "Find the shortest path between two nodes using Dijkstra's algorithm"},
    {NULL}
};
// clang-format on

static PyObject* Graph_get_node_count(GraphObject* self,
                                      PyObject*    Py_UNUSED(ignored))
{
    return PyLong_FromSsize_t(self->node_count);
}

static PyObject* Graph_add_node(GraphObject* self, PyObject* Py_UNUSED(ignored))
{
    if (self->node_count >= self->max_node_count)
    {
        // approximately a growth factor of 112.5%
        self->max_node_count =
            (self->max_node_count + (self->max_node_count >> 3) + 6) &
            (~(Py_ssize_t) 3);
        self->adj_list = realloc(self->adj_list, sizeof(*self->adj_list) *
                                                     self->max_node_count);
        if (self->adj_list == NULL)
        {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc adj_list at memory address %p",
                         (void*) self->adj_list);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i)
        {
            self->adj_list[i] = NULL;
        }

        self->neighbor_count =
            realloc(self->neighbor_count,
                    sizeof(*self->neighbor_count) * self->max_node_count);
        if (self->neighbor_count == NULL)
        {
            PyErr_Format(
                PyExc_MemoryError,
                "Unable to realloc neighbor_count at memory address %p",
                (void*) self->neighbor_count);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i)
        {
            self->neighbor_count[i] = 0;
        }

        self->max_neighbor_count =
            realloc(self->max_neighbor_count,
                    sizeof(*self->max_neighbor_count) * self->max_node_count);
        if (self->max_neighbor_count == NULL)
        {
            PyErr_Format(
                PyExc_MemoryError,
                "Unable to realloc max_neighbor_count at memory address %p",
                (void*) self->max_neighbor_count);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i)
        {
            self->max_neighbor_count[i] = 0;
        }
    }

    return PyLong_FromSsize_t(self->node_count++);
}

static PyObject* Graph_add_edge(GraphObject* self, PyObject* args,
                                PyObject* kwds)
{
    static char* kwlist[] = {"u", "v", NULL};
    Py_ssize_t   u, v;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "nn", kwlist, &u, &v))
    {
        return NULL;
    }

    if (u < 0 || u >= self->node_count || v < 0 || v >= self->node_count)
    {
        PyErr_Format(PyExc_ValueError,
                     "u and v should be existing nodes. Graph has "
                     "node_count=%ld but given u=%ld and v=%ld",
                     self->node_count, u, v);
        return NULL;
    }

    if (self->neighbor_count[u] >= self->max_neighbor_count[u])
    {
        self->max_neighbor_count[u] = (self->max_neighbor_count[u] +
                                       (self->max_neighbor_count[u] >> 3) + 6) &
                                      (~(Py_ssize_t) 3);
        self->adj_list[u] =
            realloc(self->adj_list[u],
                    sizeof(*self->adj_list[u]) * self->max_neighbor_count[u]);
        if (self->adj_list[u] == NULL)
        {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc adj_list[u] at memory address %p "
                         "with u=%ld",
                         (void*) self->adj_list[u], u);
            return NULL;
        }
    }
    self->adj_list[u][self->neighbor_count[u]++] = v;

    if (self->neighbor_count[v] >= self->max_neighbor_count[v])
    {
        self->max_neighbor_count[v] = (self->max_neighbor_count[v] +
                                       (self->max_neighbor_count[v] >> 3) + 6) &
                                      (~(Py_ssize_t) 3);
        self->adj_list[v] =
            realloc(self->adj_list[v],
                    sizeof(*self->adj_list[v]) * self->max_neighbor_count[v]);
        if (self->adj_list[v] == NULL)
        {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc adj_list[v] at memory address %p "
                         "with v=%ld",
                         (void*) self->adj_list[v], v);
            return NULL;
        }
    }
    self->adj_list[v][self->neighbor_count[v]++] = u;

    Py_RETURN_NONE;
}

static PyObject* Graph_dijkstra_path(GraphObject* self, PyObject* args,
                                     PyObject* kwds)
{
    static char* kwlist[] = {"src", "dst", "weight", NULL};
    Py_ssize_t   src, dst;
    PyObject*    weight = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "nnO", kwlist, &src, &dst,
                                     &weight))
    {
        return NULL;
    }

    if (!PyCallable_Check(weight))
    {
        PyErr_SetString(PyExc_TypeError, "weight must be callable.");
        return NULL;
    }

    Py_ssize_t* dist = malloc(sizeof(*dist) * self->node_count);
    if (dist == NULL)
    {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc dist at memory address %p",
                     (void*) dist);
        return NULL;
    }

    short* visited = malloc(sizeof(*visited) * self->node_count);
    if (visited == NULL)
    {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc visited at memory address %p",
                     (void*) visited);
        return NULL;
    }

    Py_ssize_t* prev = malloc(sizeof(*prev) * self->node_count);
    if (prev == NULL)
    {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc prev at memory address %p",
                     (void*) prev);
        return NULL;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i)
    {
        dist[i] = PY_SSIZE_T_MAX;
        visited[i] = 0;
        prev[i] = self->node_count;
    }
    dist[src] = 0;
    visited[src] = 1;
    prev[src] = src;

    MinHeap* heap =
        MinHeap_alloc((self->node_count * (self->node_count - 1)) / 2);
    MinHeap_insert(heap, src, 0);
    Py_ssize_t u, v;
    double     w;
    PyObject*  uvargs;
    PyObject*  ret_value;
    while (!MinHeap_is_empty(heap))
    {
        u = MinHeap_extract_min(heap);
        visited[u] = 1;
        for (Py_ssize_t i = 0; i < self->neighbor_count[u]; ++i)
        {
            v = self->adj_list[u][i];
            if (visited[v])
            {
                continue;
            }
            uvargs = Py_BuildValue("(nn)", u, v);
            if (uvargs == NULL)
            {
                PyErr_Format(PyExc_TypeError,
                             "Unable to format args given u=%ld and v=%ld", u,
                             v);
                return NULL;
            }
            ret_value = PyObject_Call(weight, uvargs, NULL);
            if (ret_value == NULL)
            {
                PyErr_Format(PyExc_TypeError,
                             "Unable to call weight function on args given "
                             "weight=%R and args=%R",
                             weight, args);
                return NULL;
            }
            w = PyFloat_AsDouble(ret_value);
            if (w == -1 && PyErr_Occurred() != NULL)
            {
                PyErr_Format(PyExc_ValueError,
                             "weight function returned a non-float value "
                             "given ret_value=%R",
                             ret_value);
                return NULL;
            }

            if (dist[v] - dist[u] > w)
            {
                dist[v] = dist[u] + w;
                prev[v] = u;
                MinHeap_insert(heap, v, dist[v]);
            }
        }
    }

    PyObject* path = PyList_New(0);
    if (path == NULL)
    {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize path");
    }
    if (prev[dst] == self->node_count)
    {
        return path;
    }

    if (PyList_Append(path, PyLong_FromSsize_t(dst)) == -1)
    {

        return NULL;
    }
    Py_ssize_t curr = dst;
    do
    {
        curr = prev[curr];
        if (PyList_Append(path, PyLong_FromSsize_t(curr)) == -1)
        {

            return NULL;
        }
    } while (curr != src);

    if (PyList_Reverse(path) == -1)
    {
        return NULL;
    }

    free(dist);
    dist = NULL;
    free(visited);
    visited = NULL;
    free(prev);
    prev = NULL;
    MinHeap_free(heap);
    heap = NULL;

    return path;
}
