#include <Python.h>
#include <structmember.h>

#include "accimage.h"

static struct PyMemberDef Image_members[] = {
    { "channels", T_INT, offsetof(ImageObject, channels), READONLY, "number of channels" },
    { "height",   T_INT, offsetof(ImageObject, height),   READONLY, "image height in pixels" },
    { "width",    T_INT, offsetof(ImageObject, width),    READONLY, "image width in pixels" },
    { NULL }  /* Sentinel */
};

static PyObject* Image_getsize(ImageObject* self, void* closure) {
    return Py_BuildValue("ii", self->width, self->height);
}

static PyGetSetDef Image_getseters[] = {
    { "size", (getter) Image_getsize, (setter) NULL, "Image width x height", NULL },
    { NULL }  /* Sentinel */
};

static PyObject* Image_resize(ImageObject* self, PyObject* args, PyObject* kwds) {
    static char* argnames[] = { "size", "interpolation", NULL };
    PyObject* size = NULL;
    int interpolation = 0;
    int antialiasing = 1;
    int new_height, new_width;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", argnames, &size, &interpolation)) {
        return NULL;
    }

    if (!PyArg_ParseTuple(size, "ii", &new_height, &new_width)) {
        return NULL;
    }

    if (new_height <= 0) {
        return PyErr_Format(PyExc_ValueError, "positive height expected; got %d", new_height);
    }

    if (new_width <= 0) {
        return PyErr_Format(PyExc_ValueError, "positive width expected; got %d", new_width);
    }

    // TODO: consider interpolation parameter
    image_resize(self, new_height, new_width, antialiasing);

    if (PyErr_Occurred()) {
        return NULL;
    } else {
        Py_INCREF(self);
        return (PyObject*) self;
    }
}

static PyObject* Image_crop(ImageObject* self, PyObject* args, PyObject* kwds) {
    static char* argnames[] = { "box", NULL };
    PyObject* box_object;
    int left, upper, right, lower;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", argnames, &box_object)) {
        return NULL;
    }

    if (!PyArg_ParseTuple(box_object, "iiii", &left, &upper, &right, &lower)) {
        return NULL;
    }

    if (left < 0) {
        return PyErr_Format(PyExc_ValueError, "non-negative left offset expected; got %d", left);
    }

    if (upper < 0) {
        return PyErr_Format(PyExc_ValueError, "non-negative upper offset expected; got %d", upper);
    }

    if (right > self->width) {
        return PyErr_Format(PyExc_ValueError, "right coordinate (%d) extends beyond image width (%d)",
            right, self->width);
    }

    if (lower > self->height) {
        return PyErr_Format(PyExc_ValueError, "lower coordinate (%d) extends beyond image height (%d)",
            lower, self->height);
    }

    if (right <= left) {
        return PyErr_Format(PyExc_ValueError, "right coordinate (%d) does not exceed left coordinate (%d)",
            right, left);
    }

    if (lower <= upper) {
        return PyErr_Format(PyExc_ValueError, "lower coordinate (%d) does not exceed upper coordinate (%d)",
            lower, upper);
    }

    self->y_offset += upper;
    self->x_offset += left;
    self->height = lower - upper;
    self->width = right - left;

    Py_INCREF(self);
    return (PyObject*) self;
}

static PyObject* Image_transpose(ImageObject* self, PyObject* args, PyObject* kwds) {
    static char* argnames[] = { "method", NULL };
    int method;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", argnames, &method))
        return NULL;

    switch (method) {
        case 0:
            /* PIL.Image.FLIP_LEFT_RIGHT */
            image_flip_left_right(self);
            break;
        case 1:
            PyErr_SetString(PyExc_ValueError, "unsupported method: PIL.Image.FLIP_TOP_BOTTOM");
            return NULL;
        case 2:
            PyErr_SetString(PyExc_ValueError, "unsupported method: PIL.Image.ROTATE_90");
            return NULL;
        case 3:
            PyErr_SetString(PyExc_ValueError, "unsupported method: PIL.Image.ROTATE_180");
            return NULL;
        case 4:
            PyErr_SetString(PyExc_ValueError, "unsupported method: PIL.Image.ROTATE_270");
            return NULL;
        case 5:
            PyErr_SetString(PyExc_ValueError, "unsupported method: PIL.Image.TRANSPOSE");
            return NULL;
        default:
            return PyErr_Format(PyExc_ValueError, "unknown method (%d)", method);
    }

    if (PyErr_Occurred()) {
        return NULL;
    } else {
        Py_INCREF(self);
        return (PyObject*) self;
    }
}

static PyObject* Image_copyto(ImageObject* self, PyObject* args, PyObject* kwds) {
    static char* argnames[] = { "buffer", NULL };
    static const int FLAGS = PyBUF_CONTIG | PyBUF_FORMAT;
    PyObject* buffer_object;
    Py_buffer buffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", argnames, &buffer_object)) {
        return NULL;
    }

    if (PyObject_GetBuffer(buffer_object, &buffer, FLAGS) < 0) {
        return NULL;
    }

    int expected_size = self->channels * self->height * self->width;
    if (strcmp(buffer.format, "f") == 0) {
      expected_size *= sizeof(float);
    }

    if (buffer.len < expected_size) {
        PyErr_Format(PyExc_IndexError, "buffer size (%lld) is smaller than image size (%d)",
            (long long) buffer.len, expected_size);
        goto cleanup;
    }

    if (buffer.format == NULL || strcmp(buffer.format, "B") == 0) {
        image_copy_deinterleave(self, (unsigned char*) buffer.buf);
    } else if (strcmp(buffer.format, "f") == 0) {
        image_copy_deinterleave_float(self, (float*) buffer.buf);
    } else {
        PyErr_SetString(PyExc_TypeError, "buffer of unsigned byte or float elements expected");
        goto cleanup;
    }


cleanup:
    PyBuffer_Release(&buffer);

    if (PyErr_Occurred()) {
        return NULL;
    } else {
        Py_RETURN_NONE;
    }
}

static PyMethodDef Image_methods[] = {
    { "resize",    (PyCFunction) Image_resize,    METH_VARARGS | METH_KEYWORDS, "Scale image to new size." },
    { "crop",      (PyCFunction) Image_crop,      METH_VARARGS | METH_KEYWORDS, "Crop image to new size." },
    { "transpose", (PyCFunction) Image_transpose, METH_VARARGS | METH_KEYWORDS, "Transpose/flip/rotate image." },
    { "copyto",    (PyCFunction) Image_copyto,    METH_VARARGS | METH_KEYWORDS, "Copy data to a buffer." },
    { NULL }  /* Sentinel */
};

static PyTypeObject Image_Type = {
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(NULL, 0)
    "accimage.Image",           /*tp_name*/
    sizeof(ImageObject),        /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    /* methods */
    0, /* see initaccimage */   /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    0,                          /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    0,                          /*tp_iter*/
    0,                          /*tp_iternext*/
    Image_methods,              /*tp_methods*/
    Image_members,              /*tp_members*/
    Image_getseters,            /*tp_getset*/
    0, /* see initaccimage */   /*tp_base*/
    0,                          /*tp_dict*/
    0,                          /*tp_descr_get*/
    0,                          /*tp_descr_set*/
    0,                          /*tp_dictoffset*/
    0, /* see initaccimage */   /*tp_init*/
    0,                          /*tp_alloc*/
    0, /* see initaccimage */   /*tp_new*/
    0,                          /*tp_free*/
    0,                          /*tp_is_gc*/
};

static int Image_init(ImageObject *self, PyObject *args, PyObject *kwds) {
    static char* argnames[] = { "path", NULL };
    const char *path;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", argnames, &path))
        return -1;

    image_from_jpeg(self, path);

    return PyErr_Occurred() ? -1 : 0;
}

static void Image_dealloc(ImageObject *self) {
    if (self->buffer != NULL) {
        free(self->buffer);
        self->buffer = NULL;
    }
}

#if PY_MAJOR_VERSION == 2
PyMODINIT_FUNC initaccimage(void) {
#else
PyMODINIT_FUNC PyInit_accimage(void) {
#endif
    PyObject* m;

    /*
     * Due to cross platform compiler issues the slots must be filled here.
     * It's required for portability to Windows without requiring C++.
     */
    Image_Type.tp_base = &PyBaseObject_Type;
    Image_Type.tp_init = (initproc) Image_init;
    Image_Type.tp_dealloc = (destructor) Image_dealloc;
    Image_Type.tp_new = PyType_GenericNew;

#if PY_MAJOR_VERSION == 2
    m = Py_InitModule("accimage", NULL);
#else
    static struct PyModuleDef module_def = {
       PyModuleDef_HEAD_INIT,
       "accimage",
       NULL,
       -1,
       NULL
    };
    m = PyModule_Create(&module_def);
#endif

    if (m == NULL)
        goto err;

    if (PyType_Ready(&Image_Type) < 0)
        goto err;

    PyModule_AddObject(m, "Image", (PyObject*) &Image_Type);

#if PY_MAJOR_VERSION == 2
    return;
#else
    return m;
#endif

err:
#if PY_MAJOR_VERSION == 2
    return;
#else
    return NULL;
#endif
}
