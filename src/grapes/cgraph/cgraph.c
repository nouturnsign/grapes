#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cgraph.h"

#include <numpy/arrayobject.h>

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

    import_array();
    if (PyErr_Occurred()) {
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
    .m_doc = PyDoc_STR("Contains the Multigraph class, implementing "
                       "algorithms and data structures in C."),
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
    {"get_weights", (PyCFunction) Multigraph_get_weights, METH_NOARGS,
     "Get the weights in the graph."},
    {"add_node", (PyCFunction) Multigraph_add_node, METH_NOARGS,
     "Add a node to the graph, returning the newest node."},
    {"add_edge", (PyCFunction) Multigraph_add_edge,
     METH_VARARGS | METH_KEYWORDS,
     "Add an undirected edge to the graph given existing nodes."},
    {"dijkstra", (PyCFunction) Multigraph_dijkstra,
     METH_VARARGS | METH_KEYWORDS, "Multiple source Dijkstra's algorithm."},
    {"floyd_warshall", (PyCFunction) Multigraph_floyd_warshall, METH_NOARGS,
     "Floyd-Warshall algorithm."},
    {"get_component_sizes", (PyCFunction) Multigraph_get_component_sizes,
     METH_NOARGS, "Return the sizes of the components in the graph."},
    {"is_bipartite", (PyCFunction) Multigraph_is_bipartite, METH_NOARGS,
     "Return whether the graph is bipartite or not."},
    {"compute_circular_layout",
     (PyCFunction) Multigraph_compute_circular_layout,
     METH_VARARGS | METH_KEYWORDS, "Compute a circular layout for the graph."},
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
Multigraph_get_weights(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject *weights = PyList_New(self->edge_count);
    if (weights == NULL) {
        PyErr_SetString(PyExc_MemoryError,
                        "Unable to initialize weights list");
    }

    Py_ssize_t i = 0;
    PyObject  *weight;
    for (Py_ssize_t u = 0; u < self->node_count; ++u) {
        for (Py_ssize_t j = 0; j < self->neighbor_count[u]; ++j) {
            Py_ssize_t v = self->adj_list[u][j];
            if (!self->is_directed && u > v) {
                continue;
            }
            double w = self->weight[u][j];
            weight = PyFloat_FromDouble(w);
            if (weight == NULL) {
                PyErr_Format(PyExc_TypeError,
                             "Unable to format weight given w=%f", w);
                return NULL;
            }
            if (PyList_SetItem(weights, i, weight) == -1) {
                return NULL;
            }
            ++i;
        }
    }
    return weights;
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
Multigraph_dijkstra(MultigraphObject *self, PyObject *args, PyObject *kwds)
{
    PyObject   *retvalue = NULL;
    Py_ssize_t *srcs = NULL;
    double     *dist = NULL;
    Py_ssize_t *prev = NULL;

    static char *kwlist[] = {"srcs", "dst", NULL};
    PyObject    *srcs_list;
    Py_ssize_t   dst;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "On", kwlist, &srcs_list,
                                     &dst)) {
        goto err;
    }

    Py_ssize_t src_count = PyList_Size(srcs_list);
    srcs = malloc(sizeof(*srcs) * src_count);
    if (srcs == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc srcs at memory address %p",
                     (void *) srcs);
        goto err;
    }
    for (Py_ssize_t i = 0; i < src_count; ++i) {
        PyObject *src = PyList_GetItem(srcs_list, i);
        if (src == NULL) {
            goto err;
        }
        srcs[i] = PyLong_AsSsize_t(src);
    }

    dist = malloc(sizeof(*dist) * self->node_count);
    if (dist == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc dist at memory address %p",
                     (void *) dist);
        goto err;
    }

    prev = malloc(sizeof(*prev) * self->node_count);
    if (prev == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc prev at memory address %p",
                     (void *) prev);
        goto err;
    }

    visit_dijkstra(self->adj_list, self->neighbor_count, self->node_count,
                   srcs, src_count, self->weight, dist, prev);
    if (PyErr_Occurred()) {
        goto err;
    }
    free(srcs);
    srcs = NULL;

    PyObject *dist_list = PyList_New(self->node_count);
    if (dist_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize dist_list");
        goto err;
    }
    PyObject *prev_list = PyList_New(self->node_count);
    if (prev_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize prev_list");
        goto err;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        PyList_SET_ITEM(dist_list, i, PyFloat_FromDouble(dist[i]));
        PyList_SET_ITEM(prev_list, i, PyLong_FromSsize_t(prev[i]));
    }

    retvalue = Py_BuildValue("(OO)", dist_list, prev_list);
err:
    free(srcs);
    free(dist);
    free(prev);
    return retvalue;
}

static PyObject *
Multigraph_floyd_warshall(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject    *retvalue;
    double     **dist = NULL;
    Py_ssize_t **prev = NULL;

    dist = malloc(sizeof(*dist) * self->node_count);
    if (dist == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc dist at memory address %p",
                     (void *) dist);
        goto err;
    }
    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        dist[i] = NULL;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        dist[i] = malloc(sizeof(*dist[i]) * self->node_count);
        if (dist[i] == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to malloc dist[i] at memory address %p",
                         (void *) dist[i]);
            goto err;
        }
        for (Py_ssize_t j = 0; j < self->node_count; ++j) {
            dist[i][j] = INFINITY;
        }
    }

    prev = malloc(sizeof(*prev) * self->node_count);
    if (prev == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc prev at memory address %p",
                     (void *) prev);
        goto err;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        prev[i] = NULL;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        prev[i] = malloc(sizeof(*prev[i]) * self->node_count);
        if (prev[i] == NULL) {
            PyErr_Format(PyExc_MemoryError,
                         "Unable to malloc prev[i] at memory address %p",
                         (void *) prev[i]);
            goto err;
        }
        for (Py_ssize_t j = 0; j < self->node_count; ++j) {
            prev[i][j] = self->node_count;
        }
    }

    for (Py_ssize_t u = 0; u < self->node_count; ++u) {
        dist[u][u] = 0;
        prev[u][u] = u;
        for (Py_ssize_t j = 0; j < self->neighbor_count[u]; ++j) {
            Py_ssize_t v = self->adj_list[u][j];
            dist[u][v] = self->weight[u][j];
            prev[u][v] = u;
        }
    }

    for (Py_ssize_t k = 0; k < self->node_count; ++k) {
        for (Py_ssize_t i = 0; i < self->node_count; ++i) {
            for (Py_ssize_t j = 0; j < self->node_count; ++j) {
                if (dist[i][j] > dist[i][k] + dist[k][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    prev[i][j] = prev[k][j];
                }
            }
        }
    }

    PyObject *dist_list = PyList_New(self->node_count);
    if (dist_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize dist_list");
        goto err;
    }

    PyObject *prev_list = PyList_New(self->node_count);
    if (prev_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to initialize prev_list");
        goto err;
    }

    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        PyObject *dist_row = PyList_New(self->node_count);
        if (dist_row == NULL) {
            PyErr_SetString(PyExc_MemoryError,
                            "Unable to initialize dist_row");
            goto err;
        }
        PyObject *prev_row = PyList_New(self->node_count);
        if (prev_row == NULL) {
            PyErr_SetString(PyExc_MemoryError,
                            "Unable to initialize prev_row");
            goto err;
        }

        for (Py_ssize_t j = 0; j < self->node_count; ++j) {
            PyList_SET_ITEM(dist_row, j, PyFloat_FromDouble(dist[i][j]));
            PyList_SET_ITEM(prev_row, j, PyLong_FromSsize_t(prev[i][j]));
        }
        PyList_SET_ITEM(dist_list, i, dist_row);
        PyList_SET_ITEM(prev_list, i, prev_row);
    }

    retvalue = Py_BuildValue("(OO)", dist_list, prev_list);
err:
    if (*dist) {
        for (Py_ssize_t u = 0; u < self->node_count; ++u) {
            free(dist[u]);
        }
    }
    free(dist);
    if (*prev) {
        for (Py_ssize_t u = 0; u < self->node_count; ++u) {
            free(prev[u]);
        }
    }
    free(prev);
    return retvalue;
}

static PyObject *
Multigraph_get_component_sizes(MultigraphObject *self,
                               PyObject         *Py_UNUSED(ignored))
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
        if (PyErr_Occurred()) {
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
Multigraph_is_bipartite(MultigraphObject *self, PyObject *Py_UNUSED(ignored))
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
        if (PyErr_Occurred()) {
            free(color);
            return NULL;
        }
    }

    free(color);
    Py_RETURN_TRUE;
}

static PyObject *
Multigraph_compute_circular_layout(MultigraphObject *self, PyObject *args,
                                   PyObject *kwds)
{
    static char *kwlist[] = {"radius", "initial_angle", "x_center", "y_center",
                             NULL};
    double       radius;
    double       initial_angle;
    double       x_center;
    double       y_center;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "dddd", kwlist, &radius,
                                     &initial_angle, &x_center, &y_center)) {
        return NULL;
    }

    npy_float32 *raw_layout =
        malloc(sizeof(*raw_layout) * self->node_count * 2);
    if (raw_layout == NULL) {
        PyErr_Format(PyExc_MemoryError,
                     "Unable to malloc raw_layout at memory address %p",
                     (void *) raw_layout);
        return NULL;
    }
    for (Py_ssize_t i = 0; i < self->node_count; ++i) {
        double theta =
            ((double) i / self->node_count) * 2 * M_PI + initial_angle;
        raw_layout[i * 2 + 0] = radius * cos(theta) + x_center;
        raw_layout[i * 2 + 1] = radius * sin(theta) + y_center;
    }

    const npy_intp dims[2] = {self->node_count, 2};
    PyObject      *layout = PyArray_SimpleNewFromData(2, &dims[0], NPY_FLOAT32,
                                                      (void *) raw_layout);
    if (PyErr_Occurred()) {
        free(raw_layout);
        return NULL;
    }
    free(raw_layout);
    return layout;
}
