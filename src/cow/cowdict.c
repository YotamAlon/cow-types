#include "cowdict.h"

typedef struct { PyObject_HEAD PyObject *data; } CowDictObject;

static PyTypeObject CowDictType;

static int cowdict_mutate(CowDictObject *self, PyObject *new_data) {
    CowDictObject *nc = PyObject_New(CowDictObject, &CowDictType);
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

static PyObject *CowDict_new(PyTypeObject *t, PyObject *a, PyObject *k) {
    CowDictObject *self = (CowDictObject *)t->tp_alloc(t, 0);
    if (self) {
        self->data = PyDict_New();
    }
    return (PyObject *)self;
}

static int CowDict_init(CowDictObject *self, PyObject *args, PyObject *kwds) {
    PyObject *arg = NULL;
    if (!PyArg_ParseTuple(args, "|O", &arg)) return -1;

    PyObject *new_data;
    if (!arg) {
        new_data = PyDict_New();
        if (!new_data) {
            return -1;
        }
    } else if (Py_IS_TYPE(arg, &CowDictType)) {
        new_data = ((CowDictObject *)arg)->data;
        Py_INCREF(new_data);
    } else {
        new_data = PyDict_New();
        if (!new_data) {
            return -1;
        }
        if (PyDict_Update(new_data, arg) < 0) {
            Py_DECREF(new_data);
            return -1;
        }
    }

    if (kwds && PyDict_GET_SIZE(kwds)) {
        if (arg && Py_IS_TYPE(arg, &CowDictType)) {
            PyObject *copy = PyDict_Copy(new_data);
            Py_DECREF(new_data);
            if (!copy) {
                return -1;
            }
            new_data = copy;
        }
        if (PyDict_Update(new_data, kwds) < 0) {
            Py_DECREF(new_data);
            return -1;
        }
    }

    Py_XDECREF(self->data);
    self->data = new_data;
    return 0;
}

static void CowDict_dealloc(CowDictObject *self) {
    Py_XDECREF(self->data);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *CowDict_repr(CowDictObject *self) {
    PyObject *dr = PyObject_Repr(self->data);
    if (!dr) {
        return NULL;
    }
    PyObject *r = PyUnicode_FromFormat("cowdict(%U)", dr);
    Py_DECREF(dr);
    return r;
}

static Py_ssize_t CowDict_length(CowDictObject *self) {
    return PyDict_GET_SIZE(self->data);
}

static PyObject *CowDict_subscript(CowDictObject *self, PyObject *key) {
    return PyObject_GetItem(self->data, key);
}

static int CowDict_ass_subscript(CowDictObject *self, PyObject *key, PyObject *value) {
    PyObject *new_data = PyDict_Copy(self->data);
    if (!new_data) {
        return -1;
    }

    int r = value ? PyDict_SetItem(new_data, key, value) : PyObject_DelItem(new_data, key);
    if (r < 0) {
        Py_DECREF(new_data);
        return -1;
    }
    return cowdict_mutate(self, new_data);
}

static int CowDict_contains(CowDictObject *self, PyObject *key) {
    return PyDict_Contains(self->data, key);
}

static PyObject *CowDict_iter(CowDictObject *self) {
    return PyObject_GetIter(self->data);
}

static PyObject *CowDict_richcompare(CowDictObject *self, PyObject *other, int op) {
    PyObject *od = Py_IS_TYPE(other, &CowDictType) ? ((CowDictObject *)other)->data : other;
    return PyObject_RichCompare(self->data, od, op);
}

static PyObject *CowDict_get(CowDictObject *self, PyObject *args) {
    PyObject *key, *def = Py_None;
    if (!PyArg_ParseTuple(args, "O|O", &key, &def)) {
        return NULL;
    }
    PyObject *v = PyDict_GetItemWithError(self->data, key);
    if (!v) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        Py_INCREF(def);
        return def;
    }
    Py_INCREF(v);
    return v;
}

static PyObject *CowDict_keys(CowDictObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyDict_Keys(self->data);
}

static PyObject *CowDict_values(CowDictObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyDict_Values(self->data);
}

static PyObject *CowDict_items(CowDictObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyDict_Items(self->data);
}

static PyObject *CowDict_copy(CowDictObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *copy = PyDict_Copy(self->data);
    if (!copy) {
        return NULL;
    }

    CowDictObject *nc = PyObject_New(CowDictObject, &CowDictType);
    if (!nc) {
        Py_DECREF(copy);
        return NULL;
    }
    nc->data = copy;
    return (PyObject *)nc;
}

static PyObject *CowDict_update(CowDictObject *self, PyObject *args, PyObject *kwds) {
    PyObject *new_data = PyDict_Copy(self->data);
    if (!new_data) {
        return NULL;
    }

    PyObject *arg = NULL;
    if (args && PyTuple_GET_SIZE(args)) {
        arg = PyTuple_GET_ITEM(args, 0);
    }
    if (arg && PyDict_Update(new_data, arg) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (kwds && PyDict_GET_SIZE(kwds) && PyDict_Update(new_data, kwds) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowdict_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowDict_pop(CowDictObject *self, PyObject *args) {
    PyObject *key, *def = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &key, &def)) {
        return NULL;
    }
    PyObject *val = PyDict_GetItemWithError(self->data, key);
    if (!val) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        if (!def) {
            PyErr_SetObject(PyExc_KeyError, key);
            return NULL;
        }
        Py_INCREF(def); {
            return def;
        }
    }
    Py_INCREF(val);
    PyObject *new_data = PyDict_Copy(self->data);
    if (!new_data) {
        Py_DECREF(val);
        return NULL;
    }
    if (PyDict_DelItem(new_data, key) < 0) {
        Py_DECREF(new_data);
        Py_DECREF(val);
        return NULL;
    }
    if (cowdict_mutate(self, new_data) < 0) {
        Py_DECREF(val);
        return NULL;
    }
    return val;
}

static PyObject *CowDict_popitem(CowDictObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *new_data = PyDict_Copy(self->data);
    if (!new_data) {
        return NULL;
    }
    PyObject *item = PyObject_CallMethodNoArgs(new_data, PyUnicode_FromString("popitem"));
    if (!item) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowdict_mutate(self, new_data) < 0) {
        Py_DECREF(item);
        return NULL;
    }
    return item;
}

static PyObject *CowDict_clear(CowDictObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *new_data = PyDict_New();
    if (!new_data) {
        return NULL;
    }
    if (cowdict_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowDict_setdefault(CowDictObject *self, PyObject *args) {
    PyObject *key, *def = Py_None;
    if (!PyArg_ParseTuple(args, "O|O", &key, &def)) {
        return NULL;
    }
    PyObject *v = PyDict_GetItemWithError(self->data, key);
    if (v) {
        Py_INCREF(v);
        return v;
    }
    if (PyErr_Occurred()) {
        return NULL;
    }
    PyObject *new_data = PyDict_Copy(self->data);
    if (!new_data) {
        return NULL;
    }
    if (PyDict_SetItem(new_data, key, def) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowdict_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_INCREF(def);
    return def;
}

static PyGetSetDef CowDict_getsetters[] = {
    {NULL}
};

static PyMethodDef CowDict_methods[] = {
    {"get",        (PyCFunction)CowDict_get,        METH_VARARGS,                 NULL},
    {"keys",       (PyCFunction)CowDict_keys,       METH_NOARGS,                  NULL},
    {"values",     (PyCFunction)CowDict_values,     METH_NOARGS,                  NULL},
    {"items",      (PyCFunction)CowDict_items,      METH_NOARGS,                  NULL},
    {"copy",       (PyCFunction)CowDict_copy,       METH_NOARGS,                  NULL},
    {"update",     (PyCFunction)CowDict_update,     METH_VARARGS | METH_KEYWORDS, NULL},
    {"pop",        (PyCFunction)CowDict_pop,        METH_VARARGS,                 NULL},
    {"popitem",    (PyCFunction)CowDict_popitem,    METH_NOARGS,                  NULL},
    {"clear",      (PyCFunction)CowDict_clear,      METH_NOARGS,                  NULL},
    {"setdefault", (PyCFunction)CowDict_setdefault, METH_VARARGS,                 NULL},
    {NULL}
};

static PyMappingMethods CowDict_as_mapping = {
    (lenfunc)CowDict_length,
    (binaryfunc)CowDict_subscript,
    (objobjargproc)CowDict_ass_subscript,
};

static PySequenceMethods CowDict_as_sequence = {
    0, 0, 0, 0, 0, 0, 0,
    (objobjproc)CowDict_contains,
};

static PyTypeObject CowDictType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name        = "cow.cowdict",
    .tp_basicsize   = sizeof(CowDictObject),
    .tp_dealloc     = (destructor)CowDict_dealloc,
    .tp_repr        = (reprfunc)CowDict_repr,
    .tp_as_sequence = &CowDict_as_sequence,
    .tp_as_mapping  = &CowDict_as_mapping,
    .tp_iter        = (getiterfunc)CowDict_iter,
    .tp_richcompare = (richcmpfunc)CowDict_richcompare,
    .tp_flags       = Py_TPFLAGS_DEFAULT,
    .tp_methods     = CowDict_methods,
    .tp_getset      = CowDict_getsetters,
    .tp_new         = CowDict_new,
    .tp_init        = (initproc)CowDict_init,
};

int CowDict_register(PyObject *module) {
    if (PyType_Ready(&CowDictType) < 0) {
        return -1;
    }
    Py_INCREF(&CowDictType);
    if (PyModule_AddObject(module, "cowdict", (PyObject *)&CowDictType) < 0) {
        Py_DECREF(&CowDictType);
        return -1;
    }
    return 0;
}
