#include "cowlist.h"

typedef struct { PyObject_HEAD PyObject *data; } CowListObject;

static PyTypeObject CowListType;

static int cowlist_mutate(CowListObject *self, PyObject *new_data) {
    CowListObject *nc = PyObject_New(CowListObject, &CowListType);
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

static PyObject *CowList_new(PyTypeObject *t, PyObject *a, PyObject *k) {
    CowListObject *self = (CowListObject *)t->tp_alloc(t, 0);
    if (self) {
        self->data = PyList_New(0);
    }
    return (PyObject *)self;
}

static int CowList_init(CowListObject *self, PyObject *args, PyObject *kwds) {
    PyObject *arg = NULL;
    if (!PyArg_ParseTuple(args, "|O", &arg)) {
        return -1;
    }
    PyObject *new_data;
    if (!arg) {
        new_data = PyList_New(0);
    } else if (Py_IS_TYPE(arg, &CowListType)) {
        new_data = ((CowListObject *)arg)->data;
        Py_INCREF(new_data);
        Py_XDECREF(self->data);
        self->data = new_data;
        return 0;
    } else {
        new_data = PySequence_List(arg);
    }
    if (!new_data) {
        return -1;
    }
    Py_XDECREF(self->data);
    self->data = new_data;
    return 0;
}

static void CowList_dealloc(CowListObject *self) {
    Py_XDECREF(self->data);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *CowList_repr(CowListObject *self) {
    PyObject *lr = PyObject_Repr(self->data);
    if (!lr) {
        return NULL;
    }
    PyObject *r = PyUnicode_FromFormat("cowlist(%U)", lr);
    Py_DECREF(lr);
    return r;
}

static Py_ssize_t CowList_length(CowListObject *self) { return PyList_GET_SIZE(self->data); }

static PyObject *CowList_item(CowListObject *self, Py_ssize_t i) {
    if (i < 0 || i >= PyList_GET_SIZE(self->data)) {
        PyErr_SetString(PyExc_IndexError, "list index out of range"); return NULL;
    }
    PyObject *v = PyList_GET_ITEM(self->data, i);
    Py_INCREF(v);
    return v;
}

static PyObject *CowList_subscript(CowListObject *self, PyObject *key) {
    return PyObject_GetItem(self->data, key);
}

static int CowList_ass_subscript(CowListObject *self, PyObject *key, PyObject *value) {
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return -1;
    }
    int r = value ? PyObject_SetItem(new_data, key, value) : PyObject_DelItem(new_data, key);
    if (r < 0) {
        Py_DECREF(new_data);
        return -1;
    }
    return cowlist_mutate(self, new_data);
}

static int CowList_contains(CowListObject *self, PyObject *item) {
    return PySequence_Contains(self->data, item);
}

static PyObject *CowList_iter(CowListObject *self) { return PyObject_GetIter(self->data); }

static PyObject *CowList_richcompare(CowListObject *self, PyObject *other, int op) {
    PyObject *od = Py_IS_TYPE(other, &CowListType) ? ((CowListObject *)other)->data : other;
    return PyObject_RichCompare(self->data, od, op);
}

static PyObject *CowList_append(CowListObject *self, PyObject *item) {
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    if (PyList_Append(new_data, item) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_extend(CowListObject *self, PyObject *other) {
    PyObject *od = Py_IS_TYPE(other, &CowListType) ? ((CowListObject *)other)->data : other;
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    PyObject *r = PyObject_CallMethodOneArg(new_data, PyUnicode_FromString("extend"), od);
    if (!r) {
        Py_DECREF(new_data);
        return NULL;
    }
    Py_DECREF(r);
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_insert(CowListObject *self, PyObject *args) {
    Py_ssize_t index; PyObject *item;
    if (!PyArg_ParseTuple(args, "nO", &index, &item)) {
        return NULL;
    }
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    if (PyList_Insert(new_data, index, item) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_remove(CowListObject *self, PyObject *item) {
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    PyObject *r = PyObject_CallMethodOneArg(new_data, PyUnicode_FromString("remove"), item);
    if (!r) {
        Py_DECREF(new_data);
        return NULL;
    }
    Py_DECREF(r);
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_pop(CowListObject *self, PyObject *args) {
    Py_ssize_t index = -1;
    if (!PyArg_ParseTuple(args, "|n", &index)) {
        return NULL;
    }
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    PyObject *idx = PyLong_FromSsize_t(index);
    PyObject *item = PyObject_CallMethodOneArg(new_data, PyUnicode_FromString("pop"), idx);
    Py_DECREF(idx);
    if (!item) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowlist_mutate(self, new_data) < 0) {
        Py_DECREF(item);
        return NULL;
    }
    return item;
}

static PyObject *CowList_clear(CowListObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *new_data = PyList_New(0);
    if (!new_data) {
        return NULL;
    }
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_sort(CowListObject *self, PyObject *args, PyObject *kwds) {
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    args = args ? args : PyTuple_New(0);
    PyObject *r = PyObject_Call( PyObject_GetAttrString(new_data, "sort"), args, kwds);
    if (!r) {
        Py_DECREF(new_data);
        return NULL;
    }
    Py_DECREF(r);
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_reverse(CowListObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *new_data = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!new_data) {
        return NULL;
    }
    if (PyList_Reverse(new_data) < 0) {
        Py_DECREF(new_data);
        return NULL;
    }
    if (cowlist_mutate(self, new_data) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *CowList_copy(CowListObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *copy = PyList_GetSlice(self->data, 0, PyList_GET_SIZE(self->data));
    if (!copy) {
        return NULL;
    }
    CowListObject *nc = PyObject_New(CowListObject, &CowListType);
    if (!nc) {
        Py_DECREF(copy);
        return NULL;
    }
    nc->data = copy;
    return (PyObject *)nc;
}

static PyObject *CowList_index(CowListObject *self, PyObject *args) {
    PyObject *item; Py_ssize_t start = 0, stop = PY_SSIZE_T_MAX;
    if (!PyArg_ParseTuple(args, "O|nn", &item, &start, &stop)) {
        return NULL;
    }
    Py_ssize_t n = PyList_GET_SIZE(self->data);
    if (stop > n) {
        stop = n;
    }
    for (Py_ssize_t i = start; i < stop; i++) {
        int eq = PyObject_RichCompareBool(PyList_GET_ITEM(self->data, i), item, Py_EQ);
        if (eq < 0) {
            return NULL;
        }
        if (eq) {
            return PyLong_FromSsize_t(i);
        }
    }
    PyErr_SetObject(PyExc_ValueError, item);
    return NULL;
}

static PyObject *CowList_count(CowListObject *self, PyObject *item) {
    Py_ssize_t count = 0, n = PyList_GET_SIZE(self->data);
    for (Py_ssize_t i = 0; i < n; i++) {
        int eq = PyObject_RichCompareBool(PyList_GET_ITEM(self->data, i), item, Py_EQ);
        if (eq < 0) {
            return NULL;
        }
        if (eq) {
            count++;
        }
    }
    return PyLong_FromSsize_t(count);
}

static PyGetSetDef CowList_getsetters[] = {
    {NULL}
};

static PyMethodDef CowList_methods[] = {
    {"append",  (PyCFunction)CowList_append,  METH_O,                       NULL},
    {"extend",  (PyCFunction)CowList_extend,  METH_O,                       NULL},
    {"insert",  (PyCFunction)CowList_insert,  METH_VARARGS,                 NULL},
    {"remove",  (PyCFunction)CowList_remove,  METH_O,                       NULL},
    {"pop",     (PyCFunction)CowList_pop,     METH_VARARGS,                 NULL},
    {"clear",   (PyCFunction)CowList_clear,   METH_NOARGS,                  NULL},
    {"sort",    (PyCFunction)CowList_sort,    METH_VARARGS | METH_KEYWORDS, NULL},
    {"reverse", (PyCFunction)CowList_reverse, METH_NOARGS,                  NULL},
    {"copy",    (PyCFunction)CowList_copy,    METH_NOARGS,                  NULL},
    {"index",   (PyCFunction)CowList_index,   METH_VARARGS,                 NULL},
    {"count",   (PyCFunction)CowList_count,   METH_O,                       NULL},
    {NULL}
};

static PySequenceMethods CowList_as_sequence = {
    (lenfunc)CowList_length, 0, 0,
    (ssizeargfunc)CowList_item,
    0, 0, 0,
    (objobjproc)CowList_contains,
};

static PyMappingMethods CowList_as_mapping = {
    (lenfunc)CowList_length,
    (binaryfunc)CowList_subscript,
    (objobjargproc)CowList_ass_subscript,
};

static PyTypeObject CowListType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name        = "cow.cowlist",
    .tp_basicsize   = sizeof(CowListObject),
    .tp_dealloc     = (destructor)CowList_dealloc,
    .tp_repr        = (reprfunc)CowList_repr,
    .tp_as_sequence = &CowList_as_sequence,
    .tp_as_mapping  = &CowList_as_mapping,
    .tp_iter        = (getiterfunc)CowList_iter,
    .tp_richcompare = (richcmpfunc)CowList_richcompare,
    .tp_flags       = Py_TPFLAGS_DEFAULT,
    .tp_methods     = CowList_methods,
    .tp_getset      = CowList_getsetters,
    .tp_new         = CowList_new,
    .tp_init        = (initproc)CowList_init,
};

int CowList_register(PyObject *module) {
    if (PyType_Ready(&CowListType) < 0) {
        return -1;
    }
    Py_INCREF(&CowListType);
    if (PyModule_AddObject(module, "cowlist", (PyObject *)&CowListType) < 0) {
        Py_DECREF(&CowListType);
        return -1;
    }
    return 0;
}
