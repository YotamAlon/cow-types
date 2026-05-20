#include "cowset.h"

typedef struct { PyObject_HEAD PyObject *data; } CowSetObject;

static PyTypeObject CowSetType;

static int cowset_mutate(CowSetObject *self, PyObject *new_data) {
    CowSetObject *nc = PyObject_New(CowSetObject, &CowSetType);
    if (!nc) {
        Py_DECREF(new_data);
        return -1;
    }
    nc->data = new_data;
    if (!cow_replace_in_frame((PyObject *)self, (PyObject *)nc)) {
        PyObject *old = self->data;
        self->data = new_data;
        Py_INCREF(new_data);
        Py_DECREF(old);
    }
    Py_DECREF(nc);
    return 0;
}

static PyObject *CowSet_new(PyTypeObject *t, PyObject *a, PyObject *k) {
    CowSetObject *self = (CowSetObject *)t->tp_alloc(t, 0);
    if (self) {
        self->data = PySet_New(NULL);
    }
    return (PyObject *)self;
}

static int CowSet_init(CowSetObject *self, PyObject *args, PyObject *kwds) {
    PyObject *arg = NULL;
    if (!PyArg_ParseTuple(args, "|O", &arg)) {
        return -1;
    }
    PyObject *new_data;
    if (!arg) {
        new_data = PySet_New(NULL);
    } else if (Py_IS_TYPE(arg, &CowSetType)) {
        new_data = ((CowSetObject *)arg)->data;
        Py_INCREF(new_data);
        Py_XDECREF(self->data);
        self->data = new_data;
        return 0;
    } else {
        new_data = PySet_New(arg);
    }
    if (!new_data) {
        return -1;
    }
    Py_XDECREF(self->data);
    self->data = new_data;
    return 0;
}

static void CowSet_dealloc(CowSetObject *self) {
    Py_XDECREF(self->data);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *CowSet_repr(CowSetObject *self) {
    PyObject *sr = PyObject_Repr(self->data);
    if (!sr) {
        return NULL;
    }
    PyObject *r = PyUnicode_FromFormat("cowset(%U)", sr);
    Py_DECREF(sr);
    return r;
}

static Py_ssize_t CowSet_length(CowSetObject *self) {
    return PySet_GET_SIZE(self->data);
}
static int CowSet_contains(CowSetObject *self, PyObject *key) {
    return PySet_Contains(self->data, key);
}
static PyObject  *CowSet_iter(CowSetObject *self) {
    return PyObject_GetIter(self->data);
}

static PyObject *CowSet_richcompare(CowSetObject *self, PyObject *other, int op) {
    PyObject *od = Py_IS_TYPE(other, &CowSetType) ? ((CowSetObject *)other)->data : other;
    return PyObject_RichCompare(self->data, od, op);
}

static PyObject *CowSet_add(CowSetObject *self, PyObject *item) {
    PyObject *new_data = PySet_New(self->data);
    if (!new_data) {
        return NULL;
    }
    if (PySet_Add(new_data, item) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowset_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowSet_discard(CowSetObject *self, PyObject *item) {
    if (!PySet_Contains(self->data, item)) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        Py_RETURN_NONE;
    }
    PyObject *new_data = PySet_New(self->data);
    if (!new_data) {
        return NULL;
    }
    if (PySet_Discard(new_data, item) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowset_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowSet_remove(CowSetObject *self, PyObject *item) {
    if (!PySet_Contains(self->data, item)) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        PyErr_SetObject(PyExc_KeyError, item); {
            return NULL;
        }
    }
    PyObject *new_data = PySet_New(self->data);
    if (!new_data) {
        return NULL;
    }
    if (PySet_Discard(new_data, item) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowset_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowSet_pop(CowSetObject *self, PyObject *Py_UNUSED(ignored)) {
    if (PySet_GET_SIZE(self->data) == 0) {
        PyErr_SetString(PyExc_KeyError, "pop from an empty set");
        return NULL;
    }
    PyObject *new_data = PySet_New(self->data);
    if (!new_data) {
        return NULL;
    }
    PyObject *item = PySet_Pop(new_data);
    if (!item) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowset_mutate(self, new_data) < 0) {
        Py_DECREF(item);
        return NULL;
    }
    return item;
}

static PyObject *CowSet_clear(CowSetObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *new_data = PySet_New(NULL);
    if (!new_data) {
        return NULL;
    }
    if (cowset_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *cowset_apply_op(CowSetObject *self, PyObject *other, const char *methname) {
    PyObject *od = Py_IS_TYPE(other, &CowSetType) ? ((CowSetObject *)other)->data : other;
    PyObject *new_data = PySet_New(self->data);
    if (!new_data) {
        return NULL;
    }
    PyObject *result = PyObject_CallMethodOneArg(new_data, PyUnicode_FromString(methname), od);
    if (!result) {
        Py_DECREF(new_data);
        return NULL;
    }
    Py_DECREF(result);
    if (cowset_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowSet_update(CowSetObject *self, PyObject *other) {
    return cowset_apply_op(self, other, "update");
}
static PyObject *CowSet_intersection_update(CowSetObject *self, PyObject *other) {
    return cowset_apply_op(self, other, "intersection_update");
}
static PyObject *CowSet_difference_update(CowSetObject *self, PyObject *other) {
    return cowset_apply_op(self, other, "difference_update");
}
static PyObject *CowSet_symmetric_difference_update(CowSetObject *self, PyObject *other) {
    return cowset_apply_op(self, other, "symmetric_difference_update");
}

static PyObject *cowset_new_result(CowSetObject *self, PyObject *other, const char *methname) {
    PyObject *od = Py_IS_TYPE(other, &CowSetType) ? ((CowSetObject *)other)->data : other;
    PyObject *result_data = PyObject_CallMethodOneArg(self->data, PyUnicode_FromString(methname), od);
    if (!result_data) {
        return NULL;
    }
    CowSetObject *nc = PyObject_New(CowSetObject, &CowSetType);
    if (!nc) {
        Py_DECREF(result_data);
        return NULL;
    }
    nc->data = result_data;
    return (PyObject *)nc;
}

static PyObject *CowSet_union(CowSetObject *self, PyObject *other)               { return cowset_new_result(self, other, "union"); }
static PyObject *CowSet_intersection(CowSetObject *self, PyObject *other)        { return cowset_new_result(self, other, "intersection"); }
static PyObject *CowSet_difference(CowSetObject *self, PyObject *other)          { return cowset_new_result(self, other, "difference"); }
static PyObject *CowSet_symmetric_difference(CowSetObject *self, PyObject *other){ return cowset_new_result(self, other, "symmetric_difference"); }

static PyObject *CowSet_copy(CowSetObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *copy = PySet_New(self->data);
    if (!copy) {
        return NULL;
    }
    CowSetObject *nc = PyObject_New(CowSetObject, &CowSetType);
    if (!nc) {
        Py_DECREF(copy);
        return NULL;
    }
    nc->data = copy;
    return (PyObject *)nc;
}

static PyGetSetDef CowSet_getsetters[] = {
    {NULL}
};

static PyMethodDef CowSet_methods[] = {
    {"add",                         (PyCFunction)CowSet_add,                         METH_O,      NULL},
    {"discard",                     (PyCFunction)CowSet_discard,                     METH_O,      NULL},
    {"remove",                      (PyCFunction)CowSet_remove,                      METH_O,      NULL},
    {"pop",                         (PyCFunction)CowSet_pop,                         METH_NOARGS, NULL},
    {"clear",                       (PyCFunction)CowSet_clear,                       METH_NOARGS, NULL},
    {"update",                      (PyCFunction)CowSet_update,                      METH_O,      NULL},
    {"intersection_update",         (PyCFunction)CowSet_intersection_update,         METH_O,      NULL},
    {"difference_update",           (PyCFunction)CowSet_difference_update,           METH_O,      NULL},
    {"symmetric_difference_update", (PyCFunction)CowSet_symmetric_difference_update, METH_O,      NULL},
    {"union",                       (PyCFunction)CowSet_union,                       METH_O,      NULL},
    {"intersection",                (PyCFunction)CowSet_intersection,                METH_O,      NULL},
    {"difference",                  (PyCFunction)CowSet_difference,                  METH_O,      NULL},
    {"symmetric_difference",        (PyCFunction)CowSet_symmetric_difference,        METH_O,      NULL},
    {"copy",                        (PyCFunction)CowSet_copy,                        METH_NOARGS, NULL},
    {NULL}
};

static PySequenceMethods CowSet_as_sequence = {
    (lenfunc)CowSet_length, 0, 0, 0, 0, 0, 0,
    (objobjproc)CowSet_contains,
};

static PyTypeObject CowSetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name        = "cow.cowset",
    .tp_basicsize   = sizeof(CowSetObject),
    .tp_dealloc     = (destructor)CowSet_dealloc,
    .tp_repr        = (reprfunc)CowSet_repr,
    .tp_as_sequence = &CowSet_as_sequence,
    .tp_iter        = (getiterfunc)CowSet_iter,
    .tp_richcompare = (richcmpfunc)CowSet_richcompare,
    .tp_flags       = Py_TPFLAGS_DEFAULT,
    .tp_methods     = CowSet_methods,
    .tp_getset      = CowSet_getsetters,
    .tp_new         = CowSet_new,
    .tp_init        = (initproc)CowSet_init,
};

int CowSet_register(PyObject *module) {
    if (PyType_Ready(&CowSetType) < 0) {
        return -1;
    }
    Py_INCREF(&CowSetType);
    if (PyModule_AddObject(module, "cowset", (PyObject *)&CowSetType) < 0) {
        Py_DECREF(&CowSetType);
        return -1;
    }
    return 0;
}
