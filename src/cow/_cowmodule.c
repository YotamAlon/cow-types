#include "cowdict.h"
#include "cowset.h"
#include "cowlist.h"

static PyModuleDef cowmodule = {
    PyModuleDef_HEAD_INIT, "_cow", NULL, -1, NULL,
};

PyMODINIT_FUNC PyInit__cow(void) // NOLINT(misc-use-internal-linkage)
{
    PyObject *m = PyModule_Create(&cowmodule);
    if (!m) {
        return NULL;
    }
    if (CowDict_register(m) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    if (CowSet_register(m) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    if (CowList_register(m) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
