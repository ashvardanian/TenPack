#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "../tenpack.h"

struct py_tenpack_t {
    tenpack_ctx_t ctx = nullptr;
    tenpack_dimensions_t dims;
};

PYBIND11_MODULE(tenpack, t) {
    t.doc() =
        "Python bindings for TenPack multimedia unpacking library.\n"
        "Supports:\n"
        "> Guess file format from MetaData.\n"
        "> Guess file property.\n"
        "> Get unpacked file in tensor.\n"
        "---------------------------------------------\n";

    pybind11::enum_<tenpack_format_t>(t, "format", "File format enumeration")
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

    pybind11::class_<py_tenpack_t>(t, "tenpack")
        .def("guess_format",
             [](py_tenpack_t& self, std::vector<uint8_t> data) {
                 tenpack_format_t format;
                 tenpack_guess_format(data.data(), data.size(), &format, &self.ctx);
                 return format;
             })
        .def("guess_dims",
             [](py_tenpack_t& self, std::vector<uint8_t> data) {
                 tenpack_format_t format;
                 tenpack_guess_dimensions(data.data(), data.size(), format, &self.dims, &self.ctx);
                 return format;
             })
        .def("unpack", [](py_tenpack_t& self, std::vector<uint8_t> data, std::vector<uint8_t> output) {
            tenpack_format_t format;
            tenpack_unpack(data.data(), data.size(), format, &self.dims, output.data(), &self.ctx);
            return format;
        });
}