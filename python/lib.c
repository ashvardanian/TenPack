#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <fcntl.h>    // `open` files
#include <sys/stat.h> // `stat` to obtain file metadata
#include <sys/mman.h> // `mmap` to read datasets faster
#include <stddef.h>   // `offsetof`
#include <unistd.h>   // `close` files

#include "tenpack.h"

static tenpack_ctx_t default_context = NULL;

static void py_api_free_globals(void*) {
    if (default_context)
        tenpack_context_free(default_context);
    default_context = NULL;
}

static PyObject* py_api_format_str(tenpack_format_t format) {
    switch (format) {
    case tenpack_bmp_k: return PyUnicode_FromString("bmp");
    case tenpack_gif_k: return PyUnicode_FromString("gif");
    case tenpack_jxr_k: return PyUnicode_FromString("jxr");
    case tenpack_png_k: return PyUnicode_FromString("png");
    case tenpack_psd_k: return PyUnicode_FromString("psd");
    case tenpack_dwg_k: return PyUnicode_FromString("dwg");
    case tenpack_ico_k: return PyUnicode_FromString("ico");
    case tenpack_jpeg_k: return PyUnicode_FromString("jpeg");
    case tenpack_jpeg2000_k: return PyUnicode_FromString("jpeg2000");
    case tenpack_wav_k: return PyUnicode_FromString("wav");
    case tenpack_avi_k: return PyUnicode_FromString("avi");
    case tenpack_mpeg4_k: return PyUnicode_FromString("mpeg4");
    default: PyErr_SetString(PyExc_RuntimeError, "Unknown format"); return NULL;
    }
}

static PyObject* py_api_shape_dict(tenpack_shape_t shape) {
    return Py_BuildValue( //
        "{s:K, s:K, s:K, s:K, s:K}",
        "frames",
        (unsigned long long)shape.frames,
        "width",
        (unsigned long long)shape.width,
        "height",
        (unsigned long long)shape.height,
        "channels",
        (unsigned long long)shape.channels,
        "bytes_per_channel",
        (unsigned long long)shape.bytes_per_channel);
}

static PyObject* py_api_format(PyObject* self, PyObject* args) {
    PyObject* py_bytes;
    char* data;
    Py_ssize_t length;

    if (!PyArg_ParseTuple(args, "O", &py_bytes))
        return NULL;

    if (!PyBytes_Check(py_bytes)) {
        PyErr_SetString(PyExc_TypeError, "Expected bytes object");
        return NULL;
    }

    if (PyBytes_AsStringAndSize(py_bytes, &data, &length) == -1)
        return NULL;

    tenpack_format_t format;
    if (!tenpack_guess_format(data, (size_t)length, &format, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the format");
        return NULL;
    }

    return py_api_format_str(format);
}

// Python function to guess file shape
static PyObject* py_api_shape(PyObject* self, PyObject* args) {
    PyObject* py_bytes;
    char* data;
    Py_ssize_t length;

    if (!PyArg_ParseTuple(args, "O", &py_bytes))
        return NULL;

    if (!PyBytes_Check(py_bytes)) {
        PyErr_SetString(PyExc_TypeError, "Expected bytes object");
        return NULL;
    }

    if (PyBytes_AsStringAndSize(py_bytes, &data, &length) == -1)
        return NULL;

    tenpack_format_t format;
    if (!tenpack_guess_format(data, (size_t)length, &format, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the format");
        return NULL;
    }

    tenpack_shape_t shape;
    if (!tenpack_guess_shape(data, (size_t)length, format, &shape, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the shape");
        return NULL;
    }

    return py_api_shape_dict(shape);
}

void resize_shape_image(tenpack_shape_t* shape, Py_ssize_t* dims) {
    shape->height = dims[0];
    shape->width = dims[1];
    shape->channels = dims[2];
}

void resize_shape_animation(tenpack_shape_t* shape, Py_ssize_t* dims) {
    shape->frames = dims[0];
    shape->height = dims[1];
    shape->width = dims[2];
    shape->channels = dims[3];
}

void resize_shape_audio(tenpack_shape_t* shape, Py_ssize_t* dims) {
    shape->width = dims[0];
}

bool set_dims_and_type_for_image(tenpack_shape_t const* shape, npy_intp* dims, int* ndims, int* type_num) {
    dims[0] = shape->height;
    dims[1] = shape->width;
    dims[2] = shape->channels;
    *ndims = 3;

    if (!shape->is_signed) {
        switch (shape->bytes_per_channel) {
        case 1: *type_num = NPY_UINT8; break;
        case 2: *type_num = NPY_UINT16; break;
        case 4: *type_num = NPY_UINT32; break;
        case 8: *type_num = NPY_UINT64; break;
        default: *type_num = -1; return false;
        }
    }
    else {
        switch (shape->bytes_per_channel) {
        case 1: *type_num = NPY_INT8; break;
        case 2: *type_num = NPY_INT16; break;
        case 4: *type_num = NPY_INT32; break;
        case 8: *type_num = NPY_INT64; break;
        default: *type_num = -1; return false;
        }
    }
    return true;
}

bool set_dims_and_type_for_animation(tenpack_shape_t const* shape, npy_intp* dims, int* ndims, int* type_num) {
    dims[0] = shape->frames;
    dims[1] = shape->height;
    dims[2] = shape->width;
    dims[3] = shape->channels;
    *ndims = 4;

    if (!shape->is_signed) {
        switch (shape->bytes_per_channel) {
        case 1: *type_num = NPY_UINT8; break;
        case 2: *type_num = NPY_UINT16; break;
        case 4: *type_num = NPY_UINT32; break;
        case 8: *type_num = NPY_UINT64; break;
        default: *type_num = -1; return false;
        }
    }
    else {
        switch (shape->bytes_per_channel) {
        case 1: *type_num = NPY_INT8; break;
        case 2: *type_num = NPY_INT16; break;
        case 4: *type_num = NPY_INT32; break;
        case 8: *type_num = NPY_INT64; break;
        default: *type_num = -1; return false;
        }
    }
    return true;
}

bool set_dims_and_type_for_audio(tenpack_shape_t const* shape, npy_intp* dims, int* ndims, int* type_num) {
    dims[0] = shape->width;
    *ndims = 1;

    switch (shape->bytes_per_channel) {
    case 1: *type_num = NPY_INT8; break;
    case 2: *type_num = NPY_INT16; break;
    case 4: *type_num = NPY_INT32; break;
    case 8: *type_num = NPY_INT64; break;
    default: *type_num = -1; return false;
    }
    return true;
}

// Python function to unpack file
static PyObject* py_api_unpack(PyObject* self, PyObject* args) {
    PyObject* py_bytes;
    char* data;
    Py_ssize_t length;

    if (!PyArg_ParseTuple(args, "O", &py_bytes))
        return NULL;

    if (!PyBytes_Check(py_bytes)) {
        PyErr_SetString(PyExc_TypeError, "Expected bytes object");
        return NULL;
    }

    if (PyBytes_AsStringAndSize(py_bytes, &data, &length) == -1)
        return NULL;

    tenpack_format_t format;
    if (!tenpack_guess_format(data, (size_t)length, &format, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the format");
        return NULL;
    }

    tenpack_shape_t shape;
    if (!tenpack_guess_shape(data, (size_t)length, format, &shape, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the shape");
        return NULL;
    }

    int type_num = -1;
    int ndims = -1;
    npy_intp dims[4];

    // Adjust `type_num`:
    switch (format) {
    case tenpack_bmp_k: // Intentional fall-through
    case tenpack_jxr_k:
    case tenpack_png_k:
    case tenpack_ico_k:
    case tenpack_jpeg_k:
    case tenpack_jpeg2000_k: set_dims_and_type_for_image(&shape, dims, &ndims, &type_num); break;
    case tenpack_gif_k: set_dims_and_type_for_animation(&shape, dims, &ndims, &type_num); break;
    case tenpack_wav_k: set_dims_and_type_for_audio(&shape, dims, &ndims, &type_num); break;
    default: PyErr_SetString(PyExc_RuntimeError, "Unsupported format type, stay tuned"); return NULL;
    }

    if (type_num == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown scalar type");
        return NULL;
    }

    PyObject* format_str = py_api_format_str(format);
    if (format_str == NULL) {
        return NULL;
    }

    PyObject* shape_dict = py_api_shape_dict(shape);
    if (shape_dict == NULL) {
        Py_XDECREF(format_str);
        return NULL;
    }

    PyObject* numpy_array = PyArray_EMPTY(ndims, dims, type_num, 0);
    if (numpy_array == NULL) {
        Py_XDECREF(format_str);
        Py_XDECREF(shape_dict);
        return NULL;
    }

    void* buffer = PyArray_DATA((PyArrayObject*)numpy_array);
    if (!tenpack_unpack(data, (size_t)length, format, &shape, buffer, &default_context)) {
        Py_XDECREF(format_str);
        Py_XDECREF(shape_dict);
        Py_XDECREF(numpy_array);
        PyErr_SetString(PyExc_RuntimeError, "Couldn't deserialize into tensor");
        return NULL;
    }

    // Create the tuple to return
    PyObject* tuple = PyTuple_Pack(3, format_str, shape_dict, numpy_array);

    // Decrease reference count for temporary objects
    Py_XDECREF(numpy_array);
    Py_XDECREF(shape_dict);
    Py_XDECREF(format_str);
    return tuple;
}

static PyObject* py_api_unpack_into(PyObject* self, PyObject* args) {
    PyObject* py_bytes;
    PyObject* npy_data;
    char* data;
    Py_ssize_t length;

    if (!PyArg_ParseTuple(args, "OO", &py_bytes, &npy_data))
        return NULL;

    if (!PyBytes_Check(py_bytes)) {
        PyErr_SetString(PyExc_TypeError, "Expected bytes object");
        return NULL;
    }

    if (PyBytes_AsStringAndSize(py_bytes, &data, &length) == -1)
        return NULL;

    tenpack_format_t format;
    if (!tenpack_guess_format(data, (size_t)length, &format, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the format");
        return NULL;
    }

    tenpack_shape_t shape;
    if (!tenpack_guess_shape(data, (size_t)length, format, &shape, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't infer the shape");
        return NULL;
    }

    Py_buffer view;
    if (PyObject_GetBuffer(npy_data, &view, PyBUF_FULL)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't get buffer");
        PyBuffer_Release(&view);
        return NULL;
    }

    if (!PyBuffer_IsContiguous(&view, 'C')) {
        PyErr_SetString(PyExc_RuntimeError, "Strides are not supported");
        PyBuffer_Release(&view);
        return NULL;
    }

    switch (format) {
    // case tenpack_bmp_k:
    // case tenpack_jxr_k:
    // case tenpack_png_k:
    // case tenpack_ico_k:
    case tenpack_jpeg_k:
    case tenpack_jpeg2000_k: resize_shape_image(&shape, view.shape); break;
    case tenpack_gif_k: resize_shape_animation(&shape, view.shape); break;
    case tenpack_wav_k: resize_shape_audio(&shape, view.shape); break;
    default: {
        PyErr_SetString(PyExc_RuntimeError, "Unsupported format type, stay tuned");
        PyBuffer_Release(&view);
        return NULL;
    }
    }

    if (!tenpack_unpack(data, (size_t)length, format, &shape, view.buf, &default_context)) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't deserialize into tensor");
        PyBuffer_Release(&view);
        return NULL;
    }

    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

// Python function to unpack multiple files
static PyObject* py_api_unpack_paths(PyObject* self, PyObject* args) {
    // This lacks an implementation for now
    Py_RETURN_NONE;
}

// Method table
static PyMethodDef TenpackMethods[] = { //
    {"format", py_api_format, METH_VARARGS, "Guess file format"},
    {"shape", py_api_shape, METH_VARARGS, "Guess file shape"},
    {"unpack", py_api_unpack, METH_VARARGS, "Unpack file"},
    {"unpack_into", py_api_unpack_into, METH_VARARGS, "Unpack the file into a tensor of the desired shape"},
    {"unpack_paths", py_api_unpack_paths, METH_VARARGS, "Unpack multiple files"},
    {NULL, NULL, 0, NULL}};

// Module definition
static struct PyModuleDef tenpack_module = { //
    PyModuleDef_HEAD_INIT,
    "tenpack",
    "Packing Media into Tensors faster!",
    -1,
    TenpackMethods,
    NULL,
    NULL,
    NULL,
    py_api_free_globals};

// Initialization function
PyMODINIT_FUNC PyInit_tenpack(void) {
    import_array();
    return PyModule_Create(&tenpack_module);
}
