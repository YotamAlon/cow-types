#ifndef COW_UTILS_H
#define COW_UTILS_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

int cow_replace_in_frame(PyObject *old_obj, PyObject *new_obj);

#endif
