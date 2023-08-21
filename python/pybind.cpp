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

size_t choose_size(py_tenpack_t const& tp) {
    switch (tp.format) {
    case tenpack_format_t::tenpack_wav_k: return tp.dims.width * tp.dims.bytes_per_channel;
    default: return tp.dims.bytes_per_channel * tp.dims.channels * tp.dims.frames * tp.dims.height * tp.dims.width;
    }
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
        .def_readwrite("bytes_per_channel", &tenpack_dimensions_t::bytes_per_channel);

    py::class_<py_tenpack_t>(t, "tenpack")
        .def(py::init<>())
        .def_readwrite("dims", &py_tenpack_t::dims)
        .def_readwrite("format", &py_tenpack_t::format)
        .def("guess_format",
             [](py_tenpack_t& self, std::string_view data) {
                 tenpack_guess_format(data.data(), data.size(), &self.format, &self.ctx);
                 return true;
             })
        .def("guess_dims",
             [](py_tenpack_t& self, std::string_view data) {
                 tenpack_guess_dimensions(data.data(), data.size(), self.format, &self.dims, &self.ctx);
                 return true;
             })
        .def("unpack",
             [](py_tenpack_t& self, std::string_view data) -> py::object {
                 bool is_signed = self.dims.is_signed;
                 size_t sz = choose_size(self);
                 py::array_t<uint8_t> out_data(sz);
                 tenpack_unpack(data.data(), data.size(), self.format, &self.dims, out_data.request().ptr, &self.ctx);

                 sz /= self.dims.bytes_per_channel;
                 switch (is_signed ? int(self.dims.bytes_per_channel) * -1 : int(self.dims.bytes_per_channel)) {
                 case 1: return out_data;
                 case 2: return out_data.cast<py::array_t<uint16_t>>();
                 case 4: return out_data.cast<py::array_t<uint32_t>>();
                 case 8: return out_data.cast<py::array_t<uint64_t>>();
                 case -1: return out_data.cast<py::array_t<int8_t>>();
                 case -2: return out_data.cast<py::array_t<int16_t>>();
                 case -4: return out_data.cast<py::array_t<int32_t>>();
                 case -8: return out_data.cast<py::array_t<int64_t>>();
                 default: return py::none {};
                 }
             })
        .def("ctx_free", [](py_tenpack_t& self) {
            tenpack_context_free(self.ctx);
            return true;
        });
}
