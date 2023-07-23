#define PY_SSIZE_T_CLEAN
#include <Python.h>

// module
PyMODINIT_FUNC            PyInit_grapes(void);
static struct PyModuleDef grapesmodule;

// classes
typedef struct GraphObject GraphObject; // clang-format on
static PyTypeObject        GraphType;
static void                Graph_dealloc(GraphObject* self);
static PyObject* Graph_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
static int       Graph_init(GraphObject* self, PyObject* args, PyObject* kwds);
static PyMethodDef
    Graph_methods[5]; // 1 more than listed below to include a sentinel value
static PyObject* Graph_get_node_count(GraphObject* self,
                                      PyObject*    Py_UNUSED(ignored));
static PyObject* Graph_add_node(GraphObject* self,
                                PyObject*    Py_UNUSED(ignored));
static PyObject* Graph_add_edge(GraphObject* self, PyObject* args,
                                PyObject* kwds);
static PyObject* Graph_dijkstra_path(GraphObject* self, PyObject* args,
                                     PyObject* kwds);
