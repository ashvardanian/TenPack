#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/cast.h>

#include "tenpack.h"

namespace py = pybind11;

struct py_tenpack_t {
    tenpack_ctx_t ctx = nullptr;
    tenpack_dimensions_t dims;
    tenpack_format_t format;
};

struct py_tensor_t {
    py::object numpy;
    void* data;
};

int guess_media_type(tenpack_format_t fmt) {
    if (fmt >= tenpack_bmp_k && fmt < tenpack_gif_k)
        return 1;
    else if (fmt >= tenpack_gif_k && fmt < tenpack_wav_k)
        return 2;
    else if (fmt >= tenpack_wav_k && fmt < tenpack_avi_k)
        return 3;
    else if (fmt >= tenpack_avi_k && fmt < tenpack_psd_k)
        return 4;
    else if (fmt >= tenpack_psd_k)
        return 5;

    return 0;
}

template <typename scalar_at>
py_tensor_t py_tensor(std::initializer_list<std::size_t> dims) {
    py::array_t<scalar_at> numpy(dims);
    void* data = numpy.request().ptr;
    return {py::object(std::move(numpy)), data};
}

py_tensor_t allocate(py_tenpack_t const& tp) {
    int expr = tp.dims.bytes_per_channel;
    if (tp.dims.is_signed)
        expr *= -1;

    int type = guess_media_type(tp.format);

    switch (type) {
    case 1: {
        switch (expr) {
        case 1: return py_tensor<uint8_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case 2: return py_tensor<uint16_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case 4: return py_tensor<uint32_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case 8: return py_tensor<uint64_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case -1: return py_tensor<int8_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case -2: return py_tensor<int16_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case -4: return py_tensor<int32_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        case -8: return py_tensor<int64_t>({tp.dims.height, tp.dims.width, tp.dims.channels});
        }
    }
    case 2: {
        switch (expr) {
        case 1: return py_tensor<uint8_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case 2: return py_tensor<uint16_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case 4: return py_tensor<uint32_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case 8: return py_tensor<uint64_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case -1: return py_tensor<int8_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case -2: return py_tensor<int16_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case -4: return py_tensor<int32_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        case -8: return py_tensor<int64_t>({tp.dims.frames, tp.dims.height, tp.dims.width, tp.dims.channels});
        }
    }
    case 3: {
        switch (tp.dims.bytes_per_channel) {
        case 1: return py_tensor<int8_t>({tp.dims.width * tp.dims.bytes_per_channel});
        case 2: return py_tensor<int16_t>({tp.dims.width * tp.dims.bytes_per_channel});
        case 4: return py_tensor<int32_t>({tp.dims.width * tp.dims.bytes_per_channel});
        case 8: return py_tensor<int64_t>({tp.dims.width * tp.dims.bytes_per_channel});
        }
    }
    }
    return {py::none {}, nullptr};
}

PYBIND11_MODULE(tenpack_module, t) {
    t.doc() =
        "Python bindings for TenPack multimedia unpacking library.\n"
        "Supports:\n"
        "> Guess file format from MetaData.\n"
        "> Guess file property.\n"
        "> Get unpacked file in tensor.\n"
        "---------------------------------------------\n";

    py::enum_<tenpack_format_t>(t, "format", "File format enumeration")
        .value("bmp", tenpack_bmp_k, "File format bmp")
        .value("gif", tenpack_gif_k, "File format gif")
        .value("jxr", tenpack_jxr_k, "File format jxr")
        .value("png", tenpack_png_k, "File format png")
        .value("psd", tenpack_psd_k, "File format psd")
        .value("dwg", tenpack_dwg_k, "File format dwg")
        .value("ico", tenpack_ico_k, "File format ico")
        .value("jpeg", tenpack_jpeg_k, "File format jpeg")
        .value("jpeg2000", tenpack_jpeg2000_k, "File format jpeg2000")
        .value("wav", tenpack_wav_k, "File format wav")
        .value("avi", tenpack_avi_k, "File format avi")
        .value("mpeg4", tenpack_mpeg4_k, "File format mpeg4")
        .export_values();

    py::class_<tenpack_dimensions_t>(t, "tenpack_dimensions")
        .def(py::init<>())
        .def_readwrite("frames", &tenpack_dimensions_t::frames)
        .def_readwrite("width", &tenpack_dimensions_t::width)
        .def_readwrite("height", &tenpack_dimensions_t::height)
        .def_readwrite("channels", &tenpack_dimensions_t::channels)
        .def_readwrite("bytes_per_channel", &tenpack_dimensions_t::bytes_per_channel)
        .def_readwrite("is_signed", &tenpack_dimensions_t::is_signed);

    py::class_<py_tenpack_t>(t, "tenpack")
        .def(py::init<>())
        .def_readwrite("dims", &py_tenpack_t::dims)
        .def_readwrite("format", &py_tenpack_t::format)
        .def("unpack",
             [](py_tenpack_t& self, std::string_view data) -> py::object {
                 tenpack_guess_format(data.data(), data.size(), &self.format, &self.ctx);
                 tenpack_guess_dimensions(data.data(), data.size(), self.format, &self.dims, &self.ctx);
                 auto tensor = allocate(self);
                 tenpack_unpack(data.data(), data.size(), self.format, &self.dims, tensor.data, &self.ctx);
                 return tensor.numpy;
             })
        .def("ctx_free", [](py_tenpack_t& self) {
            tenpack_context_free(self.ctx);
            return true;
        });
}
