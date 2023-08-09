#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cgraph.h"

#include "deque.h"
#include "heap.h"
#include "macros.h"
#include "trav.h"

PyMODINIT_FUNC
PyInit_cgraph(void)
{
    PyObject *m;
    if (PyType_Ready(&MultigraphType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&cgraphmodule);
    if (m == NULL) {
        return NULL;
    }

    Py_INCREF(&MultigraphType);
    if (PyModule_AddObject(m, "Multigraph", (PyObject *) &MultigraphType) <
        0) {
        Py_DECREF(&MultigraphType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

static struct PyModuleDef cgraphmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "cgraph",
    .m_doc = PyDoc_STR("Grapes core functionality written in C"),
    .m_size = -1,
};

typedef struct MultigraphObject {
    PyObject_HEAD
    int          is_directed;
    Py_ssize_t **adj_list;  // list of adjacency lists (adj_list[i]
    // = array of neighbors to node i)
    Py_ssize_t *neighbor_count;
    Py_ssize_t
        *max_neighbor_count;  // current maximum number of neighbors
                              // (max_neighbor_count[i] = current maximum
                              // number of neighbors allocated to node i)
    Py_ssize_t node_count;
    Py_ssize_t max_node_count;  // current maximum number of nodes allocated
    double   **weight;          // list of weight lists (by index)
    Py_ssize_t edge_count;
} MultigraphObject;

static PyTypeObject MultigraphType = {
    PyVarObject_HEAD_INIT(NULL, 0)  // clang-format off
    .tp_name = "grapes.cgraph.Multigraph",  // clang-format on
    .tp_doc = PyDoc_STR("Underlying graph type."),
    .tp_basicsize = sizeof(MultigraphObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_dealloc = (destructor) Multigraph_dealloc,
    .tp_new = Multigraph_new,
    .tp_init = (initproc) Multigraph_init,
    .tp_methods = Multigraph_methods,
};

static void
Multigraph_dealloc(MultigraphObject *self)
{
    for (Py_ssize_t i = 0; i < self->max_node_count; ++i) {
        free(self->adj_list[i]);
        self->adj_list[i] = NULL;
    }
    for (Py_ssize_t i = 0; i < self->max_node_count; ++i) {
        free(self->weight[i]);
        self->weight[i] = NULL;
    }
    free(self->adj_list);
    self->adj_list = NULL;
    free(self->neighbor_count);
    self->neighbor_count = NULL;
    free(self->max_neighbor_count);
    self->max_neighbor_count = NULL;
    free(self->weight);
    self->weight = NULL;
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
Multigraph_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    MultigraphObject *self;
    self = (MultigraphObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->adj_list = NULL;
        self->node_count = 0;
        self->max_node_count = 0;
        self->neighbor_count = NULL;
        self->max_neighbor_count = NULL;
        self->weight = NULL;
        self->edge_count = 0;
    }
    return (PyObject *) self;
}

static int
Multigraph_init(MultigraphObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"is_directed", "node_count", NULL};
    int          is_directed;
    Py_ssize_t   node_count = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "p|n", kwlist, &is_directed,
                                     &node_count)) {
        return -1;
    }

    if (node_count < 0) {
        PyErr_Format(PyExc_ValueError,
                     "node_count should be nonnegative, but given %ld",
                     node_count);
        return -1;
    }

    self->is_directed = is_directed;

    self->adj_list = malloc(sizeof(*self->adj_list) * node_count);
    if (self->adj_list == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc adj_list at memory address %p",
                     (void *) self->adj_list);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        self->adj_list[i] = NULL;
    }

    self->node_count = node_count;
    self->max_node_count = node_count;

    self->neighbor_count = malloc(sizeof(*self->neighbor_count) * node_count);
    if (self->neighbor_count == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc neighbor_count at memory address %p",
                     (void *) self->neighbor_count);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        self->neighbor_count[i] = 0;
    }

    self->max_neighbor_count =
        malloc(sizeof(*self->max_neighbor_count) * node_count);
    if (self->max_neighbor_count == NULL) {
        PyErr_Format(
            PyExc_MemoryError,
            "Unable to malloc max_neighbor_count at memory address %p",
            (void *) self->max_neighbor_count);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        self->max_neighbor_count[i] = 0;
    }

    self->weight = malloc(sizeof(*self->weight) * node_count);
    if (self->weight == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc weight at memory address %p",
                     (void *) self->weight);
        return -1;
    }
    for (Py_ssize_t i = 0; i < node_count; ++i) {
        self->weight[i] = NULL;
    }

    self->edge_count = 0;

    return 0;
}

static PyMethodDef Multigraph_methods[] = {
    {"get_node_count", (PyCFunction) Multigraph_get_node_count, METH_NOARGS,
     "Return the number of nodes in the graph."},
    {"get_edge_count", (PyCFunction) Multigraph_get_edge_count, METH_NOARGS,
     "Return the number of edges in the graph."},
    {"get_edges", (PyCFunction) Multigraph_get_edges, METH_NOARGS,
     "Return the edges in the graph."},
    {"add_node", (PyCFunction) Multigraph_add_node, METH_NOARGS,
     "Add a node to the graph, returning the newest node."},
    {"add_edge", (PyCFunction) Multigraph_add_edge,
     METH_VARARGS | METH_KEYWORDS,
     "Add an undirected edge to the graph given existing nodes."},
    {"dijkstra_path", (PyCFunction) Multigraph_dijkstra_path,
     METH_VARARGS | METH_KEYWORDS,
     "Find the shortest path between two nodes using Dijkstra's algorithm"},
    {"get_component_sizes", (PyCFunction) Multigraph_get_component_sizes,
     METH_NOARGS, "Return the sizes of the components in the graph."},
    {"is_bipartite", (PyCFunction) Multigraph_is_bipartite, METH_NOARGS,
     "Return whether the graph is bipartite or not."},
    {NULL}};

static PyObject *
Multigraph_get_node_count(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromSsize_t(self->node_count);
}

static PyObject *
Multigraph_get_edge_count(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromSsize_t(self->edge_count);
}

static PyObject *
Multigraph_get_edges(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject *edges = PyList_New(self->edge_count);
    if (edges == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize edges list");
    }

    Py_ssize_t i = 0;
    PyObject  *uv;
    for (Py_ssize_t u = 0; u < self->node_count; ++u) {
        for (Py_ssize_t j = 0; j < self->neighbor_count[u]; ++j) {
            Py_ssize_t v = self->adj_list[u][j];
            if (!self->is_directed && u > v) {
                continue;
            }
            uv = Py_BuildValue("(nn)", u, v);
            if (uv == NULL) {
                PyErr_Format(PyExc_TypeError,
                             "Unable to format uv given u=%ld and v=%ld", u,
                             v);
                return NULL;
            }
            if (PyList_SetItem(edges, i, uv) == -1) {
                return NULL;
            }
            ++i;
        }
    }
    return edges;
}

static PyObject *
Multigraph_add_node(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->node_count >= self->max_node_count) {
        // approximately a growth factor of 112.5%
        self->max_node_count =
            (self->max_node_count + (self->max_node_count >> 3) + 6) &
            (~(Py_ssize_t) 3);
        self->adj_list = realloc(
            self->adj_list, sizeof(*self->adj_list) * self->max_node_count);
        if (self->adj_list == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc adj_list at memory address %p",
                         (void *) self->adj_list);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i) {
            self->adj_list[i] = NULL;
        }

        self->weight = realloc(self->weight,
                               sizeof(*self->weight) * self->max_node_count);
        if (self->weight == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc weight at memory address %p",
                         (void *) self->weight);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i) {
            self->weight[i] = NULL;
        }

        self->neighbor_count =
            realloc(self->neighbor_count,
                    sizeof(*self->neighbor_count) * self->max_node_count);
        if (self->neighbor_count == NULL) {
            PyErr_Format(
                PyExc_MemoryError,
                "Unable to realloc neighbor_count at memory address %p",
                (void *) self->neighbor_count);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i) {
            self->neighbor_count[i] = 0;
        }

        self->max_neighbor_count =
            realloc(self->max_neighbor_count,
                    sizeof(*self->max_neighbor_count) * self->max_node_count);
        if (self->max_neighbor_count == NULL) {
            PyErr_Format(
                PyExc_MemoryError,
                "Unable to realloc max_neighbor_count at memory address %p",
                (void *) self->max_neighbor_count);
            return NULL;
        }
        for (Py_ssize_t i = self->node_count; i < self->max_node_count; ++i) {
            self->max_neighbor_count[i] = 0;
        }
    }

    return PyLong_FromSsize_t(self->node_count++);
}

static PyObject *
Multigraph_add_edge(MultigraphObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"u", "v", "weight", NULL};
    Py_ssize_t   u, v;
    double       weight = 1.0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "nn|$d", kwlist, &u, &v,
                                     &weight)) {
        return NULL;
    }

    if (u < 0 || u >= self->node_count || v < 0 || v >= self->node_count) {
        PyErr_Format(PyExc_ValueError,
                     "u and v should be existing nodes. Multigraph has "
                     "node_count=%ld but given u=%ld and v=%ld",
                     self->node_count, u, v);
        return NULL;
    }

    if (add_directed_edge_noinc(self, u, v, weight) == -1) {
        return NULL;
    }
    if (!self->is_directed) {
        if (add_directed_edge_noinc(self, v, u, weight) == -1) {
            return NULL;
        }
    }

    ++self->edge_count;

    Py_RETURN_NONE;
}

int
add_directed_edge_noinc(MultigraphObject *self, Py_ssize_t u, Py_ssize_t v,
                        double weight)
{
    if (self->neighbor_count[u] >= self->max_neighbor_count[u]) {
        self->max_neighbor_count[u] =
            (self->max_neighbor_count[u] + (self->max_neighbor_count[u] >> 3) +
             6) &
            (~(Py_ssize_t) 3);
        self->adj_list[u] =
            realloc(self->adj_list[u],
                    sizeof(*self->adj_list[u]) * self->max_neighbor_count[u]);
        if (self->adj_list[u] == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc adj_list[u] at memory address %p "
                         "with u=%ld",
                         (void *) self->adj_list[u], u);
            return -1;
        }
        self->weight[u] =
            realloc(self->weight[u],
                    sizeof(*self->weight[u]) * self->max_neighbor_count[u]);
        if (self->weight[u] == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to realloc weight[u] at memory address %p "
                         "with u=%ld",
                         (void *) self->weight[u], u);
            return -1;
        }
    }
    self->adj_list[u][self->neighbor_count[u]] = v;
    self->weight[u][self->neighbor_count[u]] = weight;
    ++self->neighbor_count[u];
    return 0;
}

static PyObject *
Multigraph_dijkstra_path(MultigraphObject *self, PyObject *args,
                         PyObject *kwds)
{
    static char *kwlist[] = {"src", "dst", NULL};
    Py_ssize_t   src, dst;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "nn", kwlist, &src, &dst)) {
        return NULL;
    }

    double *dist = malloc(sizeof(*dist) * self->node_count);
    if (dist == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc dist at memory address %p",
                     (void *) dist);
        return NULL;
    }

    Py_ssize_t *prev = malloc(sizeof(*prev) * self->node_count);
    if (prev == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc prev at memory address %p",
                     (void *) prev);
        free(dist);
        return NULL;
    }

    visit_dijkstra(self->adj_list, self->neighbor_count, self->node_count, src,
                   self->weight, dist, prev);
    if (PyErr_Occurred() != NULL) {
        free(dist);
        free(prev);
        return NULL;
    }

    PyObject *path = PyList_New(0);
    if (path == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize path");
    }
    if (prev[dst] == self->node_count) {
        free(dist);
        free(prev);
        return path;
    }

    if (PyList_Append(path, PyLong_FromSsize_t(dst)) == -1) {
        free(dist);
        free(prev);
        return NULL;
    }
    Py_ssize_t curr = dst;
    do {
        curr = prev[curr];
        if (PyList_Append(path, PyLong_FromSsize_t(curr)) == -1) {
            free(dist);
            free(prev);
            return NULL;
        }
    } while (curr != src);

    if (PyList_Reverse(path) == -1) {
        free(dist);
        free(prev);
        return NULL;
    }

    free(dist);
    free(prev);
    return path;
}

static PyObject *
Multigraph_get_component_sizes(MultigraphObject *self, PyObject *args,
                               PyObject *kwds)
{
    Py_ssize_t *sizes = malloc(sizeof(*sizes) * self->node_count);
    if (sizes == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc sizes at memory address %p",
                     (void *) sizes);
        return NULL;
    }
    short *visited = malloc(sizeof(*visited) * self->node_count);
    if (visited == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc visited at memory address %p",
                     (void *) visited);
        free(sizes);
        return NULL;
    }
    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        sizes[i] = 0;
        visited[i] = GRAPES_FALSE;
    }

    Py_ssize_t count = 0;
    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        if (!visited[i]) {
            sizes[count++] =
                visit(self->adj_list, self->neighbor_count, i, visited);
        }
        if (PyErr_Occurred() != NULL) {
            free(sizes);
            free(visited);
            return NULL;
        }
    }

    PyObject *component_sizes = PyList_New(count);
    if (component_sizes == NULL) {
        PyErr_SetString(PyExc_MemoryError,
                        "Unable to initialize component_sizes");
    }

    for (Py_ssize_t i = 0; i < count; ++i) {
        if (PyList_SetItem(component_sizes, i, PyLong_FromSsize_t(sizes[i])) ==
            -1) {
            free(sizes);
            free(visited);
            return NULL;
        }
    }

    free(sizes);
    free(visited);
    return component_sizes;
}

static PyObject *
Multigraph_is_bipartite(MultigraphObject *self, PyObject *args, PyObject *kwds)
{
    short *color = malloc(sizeof(*color) * self->node_count);
    if (color == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc color at memory address %p",
                     (void *) color);
        return NULL;
    }
    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        color[i] = GRAPES_NO_COLOR;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        if (!visit_color(self->adj_list, self->neighbor_count, i, color)) {
            free(color);
            Py_RETURN_FALSE;
        }
        if (PyErr_Occurred() != NULL) {
            free(color);
            return NULL;
        }
    }

    free(color);
    Py_RETURN_TRUE;
}
