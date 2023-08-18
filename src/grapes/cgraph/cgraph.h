#ifndef GRAPES_GRAPES_CGRAPH_CGRAPH_H_
#define GRAPES_GRAPES_CGRAPH_CGRAPH_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

// module
PyMODINIT_FUNC            PyInit_cgraph(void);
static struct PyModuleDef cgraphmodule;

// classes
typedef struct MultigraphObject MultigraphObject;
static PyTypeObject             MultigraphType;
static void                     Multigraph_dealloc(MultigraphObject *self);
static PyObject   *Multigraph_new(PyTypeObject *type, PyObject *args,
                                  PyObject *kwds);
static int         Multigraph_init(MultigraphObject *self, PyObject *args,
                                   PyObject *kwds);
static PyMethodDef Multigraph_methods[11];  // 1 more than listed below to
                                            // include a sentinel value
static PyObject *Multigraph_get_node_count(MultigraphObject *self,
                                           PyObject *Py_UNUSED(ignored));
static PyObject *Multigraph_get_edge_count(MultigraphObject *self,
                                           PyObject *Py_UNUSED(ignored));
static PyObject *Multigraph_get_edges(MultigraphObject *self,
                                      PyObject         *Py_UNUSED(ignored));
static PyObject *Multigraph_get_weights(MultigraphObject *self,
                                        PyObject         *Py_UNUSED(ignored));
static PyObject *Multigraph_add_node(MultigraphObject *self,
                                     PyObject         *Py_UNUSED(ignored));
static PyObject *Multigraph_add_edge(MultigraphObject *self, PyObject *args,
                                     PyObject *kwds);
static PyObject *Multigraph_dijkstra(MultigraphObject *self, PyObject *args,
                                     PyObject *kwds);
static PyObject *Multigraph_get_component_sizes(MultigraphObject *self,
                                                PyObject *Py_UNUSED(ignored));
static PyObject *Multigraph_is_bipartite(MultigraphObject *self,
                                         PyObject         *Py_UNUSED(ignored));
static PyObject *Multigraph_compute_circular_layout(MultigraphObject *self,
                                                    PyObject         *args,
                                                    PyObject         *kwds);

// internals
int add_directed_edge_noinc(MultigraphObject *self, Py_ssize_t u, Py_ssize_t v,
                            double weight);

#endif  // GRAPES_GRAPES_CGRAPH_CGRAPH_H_