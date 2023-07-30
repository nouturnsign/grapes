#define PY_SSIZE_T_CLEAN
#include <Python.h>

// module
PyMODINIT_FUNC            PyInit_cgraph(void);
static struct PyModuleDef cgraphmodule;

// classes
typedef struct GraphObject GraphObject;
static PyTypeObject        GraphType;
static void                Graph_dealloc(GraphObject* self);
static PyObject* Graph_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
static int       Graph_init(GraphObject* self, PyObject* args, PyObject* kwds);
static PyMethodDef
    Graph_methods[7]; // 1 more than listed below to include a sentinel value
static PyObject* Graph_get_node_count(GraphObject* self,
                                      PyObject*    Py_UNUSED(ignored));
static PyObject* Graph_get_edge_count(GraphObject* self,
                                      PyObject*    Py_UNUSED(ignored));
static PyObject* Graph_get_edges(GraphObject* self,
                                 PyObject*    Py_UNUSED(ignored));
static PyObject* Graph_add_node(GraphObject* self,
                                PyObject*    Py_UNUSED(ignored));
static PyObject* Graph_add_edge(GraphObject* self, PyObject* args,
                                PyObject* kwds);
static PyObject* Graph_dijkstra_path(GraphObject* self, PyObject* args,
                                     PyObject* kwds);

// internals
double get_weight(PyObject* weight, Py_ssize_t u, Py_ssize_t v);
