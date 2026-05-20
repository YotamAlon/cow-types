#include "cow_utils.h"

int cow_replace_in_frame(PyObject *old_obj, PyObject *new_obj)
{
    PyThreadState *tstate = PyThreadState_Get();
    PyFrameObject *frame = PyThreadState_GetFrame(tstate);
    if (!frame) {
        return 0;
    }

    PyObject *locals = PyObject_GetAttrString((PyObject *)frame, "f_locals");
    if (!locals) {
        PyErr_Clear();
        Py_DECREF(frame);
        return 0;
    }

    PyObject *items = PyMapping_Items(locals);
    if (!items) {
        PyErr_Clear();
        Py_DECREF(locals);
        Py_DECREF(frame);
        return 0;
    }

    int found = 0;
    Py_ssize_t n = PyList_GET_SIZE(items);
    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *pair = PyList_GET_ITEM(items, i);
        PyObject *key = PyTuple_GET_ITEM(pair, 0);
        PyObject *value = PyTuple_GET_ITEM(pair, 1);
        if (value == old_obj) {
            if (PyObject_SetItem(locals, key, new_obj) < 0) {
                PyErr_Clear();
            }
            else {
                found = 1;
            }
        }
    }

    Py_DECREF(items);
    Py_DECREF(locals);

#if PY_VERSION_HEX < 0x030D0000
    if (found) {
        PyFrame_LocalsToFast(frame, 0);
    }
#endif

    Py_DECREF(frame);
    return found;
}
