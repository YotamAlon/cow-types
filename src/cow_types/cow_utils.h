#ifndef COW_UTILS_H
#define COW_UTILS_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#if PY_VERSION_HEX < 0x030D0000
#include <frameobject.h>
#endif

int cow_replace_in_frame(PyObject *old_obj, PyObject *new_obj);

#endif
