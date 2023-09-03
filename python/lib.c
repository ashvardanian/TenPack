#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include <fcntl.h>    // `open` files
#include <sys/stat.h> // `stat` to obtain file metadata
#include <sys/mman.h> // `mmap` to read datasets faster
#include <stddef.h>   // `offsetof`
#include <unistd.h>   // `close` files

#include "tenpack.h"

static PyObject* py_api_format(PyObject* self, PyObject* args) {
    const char* data;
    Py_ssize_t length;
    if (!PyArg_ParseTuple(args, "s#", &data, &length))
        return NULL;

    tenpack_format_t format;
    return PyLong_FromLong((long)format);
}

// Python function to guess file shape
static PyObject* py_api_shape(PyObject* self, PyObject* args) {
    const char* data;
    Py_ssize_t length;
    if (!PyArg_ParseTuple(args, "s#", &data, &length))
        return NULL;

    tenpack_shape_t shape;
    return Py_BuildValue( //
        "{s:K, s:K, s:K, s:K, s:K, s:p}",
        "frames",
        (unsigned long long)shape.frames,
        "width",
        (unsigned long long)shape.width,
        "height",
        (unsigned long long)shape.height,
        "channels",
        (unsigned long long)shape.channels,
        "bytes_per_channel",
        (unsigned long long)shape.bytes_per_channel,
        "is_signed",
        shape.is_signed);
}

// Python function to unpack file
static PyObject* py_api_unpack(PyObject* self, PyObject* args) {
    const char* data;
    Py_ssize_t length;
    if (!PyArg_ParseTuple(args, "s#", &data, &length))
        return NULL;

    Py_RETURN_NONE;
}

// Python function to unpack multiple files
static PyObject* py_api_unpack_paths(PyObject* self, PyObject* args) {
    PyObject* path_list;
    int threads;
    if (!PyArg_ParseTuple(args, "O!i", &PyList_Type, &path_list, &threads))
        return NULL;

    Py_RETURN_NONE;
}

typedef struct {
    PyObject_HEAD tenpack_format_t format;
    tenpack_shape_t shape;
    PyObject* tensor;
    void* data;
} PyPackObject;

static PyMemberDef PyPack_members[] = {
    {"format", T_INT, offsetof(PyPackObject, format), 0, "format"},
    {"shape", T_OBJECT, offsetof(PyPackObject, shape), 0, "shape"},
    {"tensor", T_OBJECT, offsetof(PyPackObject, tensor), 0, "tensor"},
    {NULL} /* Sentinel */
};

static PyTypeObject PyPackType = {
    PyVarObject_HEAD_INIT(NULL, 0) "tenpack.Pack", /* tp_name */
    sizeof(PyPackObject),                          /* tp_basicsize */
    0,                                             /* tp_itemsize */
    0,                                             /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    0,                                             /* tp_repr */
    0,                                             /* tp_as_number */
    0,                                             /* tp_as_sequence */
    0,                                             /* tp_as_mapping */
    0,                                             /* tp_hash  */
    0,                                             /* tp_call */
    0,                                             /* tp_str */
    0,                                             /* tp_getattro */
    0,                                             /* tp_setattro */
    0,                                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                            /* tp_flags */
    "Python interface for tenpack pack",           /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    0,                                             /* tp_iter */
    0,                                             /* tp_iternext */
    0,                                             /* tp_methods */
    PyPack_members,                                /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                                             /* tp_init */
    0,                                             /* tp_alloc */
    PyType_GenericNew,                             /* tp_new */
};

// Getter functions for Shape class
static PyObject* Shape_get_frames(PyObject* self, void* closure) {
    tenpack_shape_t* shape = (tenpack_shape_t*)PyCapsule_GetPointer(self, "Shape");
    return PyLong_FromSize_t(shape->frames);
}

static PyObject* Shape_get_width(PyObject* self, void* closure) {
    tenpack_shape_t* shape = (tenpack_shape_t*)PyCapsule_GetPointer(self, "Shape");
    return PyLong_FromSize_t(shape->width);
}

static PyObject* Shape_get_height(PyObject* self, void* closure) {
    tenpack_shape_t* shape = (tenpack_shape_t*)PyCapsule_GetPointer(self, "Shape");
    return PyLong_FromSize_t(shape->height);
}

static PyObject* Shape_get_channels(PyObject* self, void* closure) {
    tenpack_shape_t* shape = (tenpack_shape_t*)PyCapsule_GetPointer(self, "Shape");
    return PyLong_FromSize_t(shape->channels);
}

static PyObject* Shape_get_bytes_per_channel(PyObject* self, void* closure) {
    tenpack_shape_t* shape = (tenpack_shape_t*)PyCapsule_GetPointer(self, "Shape");
    return PyLong_FromSize_t(shape->bytes_per_channel);
}

static PyObject* Shape_get_is_signed(PyObject* self, void* closure) {
    tenpack_shape_t* shape = (tenpack_shape_t*)PyCapsule_GetPointer(self, "Shape");
    return PyBool_FromLong(shape->is_signed);
}

// Method table for Shape class
static PyMethodDef Shape_methods[] = {
    {NULL} /* Sentinel */
};

// Property table for Shape class
static PyGetSetDef Shape_getsetters[] = {
    {"frames", (getter)Shape_get_frames, NULL, "frames", NULL},
    {"width", (getter)Shape_get_width, NULL, "width", NULL},
    {"height", (getter)Shape_get_height, NULL, "height", NULL},
    {"channels", (getter)Shape_get_channels, NULL, "channels", NULL},
    {"bytes_per_channel", (getter)Shape_get_bytes_per_channel, NULL, "bytes_per_channel", NULL},
    {"is_signed", (getter)Shape_get_is_signed, NULL, "is_signed", NULL},
    {NULL} /* Sentinel */
};

// Type specification for Shape class
static PyType_Spec ShapeTypeSpec = {
    "tenpack.Shape",         /* name */
    sizeof(tenpack_shape_t), /* basicsize */
    0,                       /* itemsize */
    Py_TPFLAGS_DEFAULT,      /* flags */
    Shape_methods,           /* methods */
    NULL,                    /* slots */
    NULL,                    /* traverse */
    NULL,                    /* clear */
    NULL,                    /* free */
    Shape_getsetters,        /* getset */
};

// Method table
static PyMethodDef TenpackMethods[] = { //
    {"format", py_api_format, METH_VARARGS, "Guess file format"},
    {"shape", py_api_shape, METH_VARARGS, "Guess file shape"},
    {"unpack", py_api_unpack, METH_VARARGS, "Unpack file"},
    {"unpack_paths", py_api_unpack_paths, METH_VARARGS, "Unpack multiple files"},
    {NULL, NULL, 0, NULL}};

// Module definition
static struct PyModuleDef tenpack_module = {
    PyModuleDef_HEAD_INIT, "tenpack", "A Python module that prints 'hello world' from C code.", -1, TenpackMethods};

// Initialization function
PyMODINIT_FUNC PyInit_tenpack(void) {

    // Create the module
    PyObject* m = PyModule_Create(&tenpack_module);
    if (m == NULL)
        return NULL;

    // Add Format enum
    PyObject* FormatEnum = PyDict_New();
    if (FormatEnum == NULL) {
        Py_DECREF(m);
        return NULL;
    }

    PyDict_SetItemString(FormatEnum, "bmp", PyLong_FromLong((long)tenpack_bmp_k));
    PyDict_SetItemString(FormatEnum, "gif", PyLong_FromLong((long)tenpack_gif_k));
    PyDict_SetItemString(FormatEnum, "jxr", PyLong_FromLong((long)tenpack_jxr_k));
    PyDict_SetItemString(FormatEnum, "png", PyLong_FromLong((long)tenpack_png_k));
    PyDict_SetItemString(FormatEnum, "psd", PyLong_FromLong((long)tenpack_psd_k));
    PyDict_SetItemString(FormatEnum, "dwg", PyLong_FromLong((long)tenpack_dwg_k));
    PyDict_SetItemString(FormatEnum, "ico", PyLong_FromLong((long)tenpack_ico_k));
    PyDict_SetItemString(FormatEnum, "jpeg", PyLong_FromLong((long)tenpack_jpeg_k));
    PyDict_SetItemString(FormatEnum, "jpeg2000", PyLong_FromLong((long)tenpack_jpeg2000_k));
    PyDict_SetItemString(FormatEnum, "wav", PyLong_FromLong((long)tenpack_wav_k));
    PyDict_SetItemString(FormatEnum, "avi", PyLong_FromLong((long)tenpack_avi_k));
    PyDict_SetItemString(FormatEnum, "mpeg4", PyLong_FromLong((long)tenpack_mpeg4_k));

    // Add Format class
    if (PyModule_AddObject(m, "Format", FormatEnum) < 0) {
        Py_DECREF(FormatEnum);
        Py_DECREF(m);
        return NULL;
    }

    // Add Shape class
    PyObject* ShapeClass = PyType_FromSpec(&ShapeTypeSpec);
    if (ShapeClass == NULL) {
        Py_DECREF(m);
        return NULL;
    }

    if (PyModule_AddObject(m, "Shape", ShapeClass) < 0) {
        Py_DECREF(ShapeClass);
        Py_DECREF(m);
        return NULL;
    }

    if (PyType_Ready(&PyPackType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&PyPackType);
    if (PyModule_AddObject(m, "Pack", (PyObject*)&PyPackType) < 0) {
        Py_DECREF(&PyPackType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
