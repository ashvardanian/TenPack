#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "../tenpack.h"

struct py_tenpack_t {
    tenpack_ctx_t ctx = nullptr;
    tenpack_dimensions_t dims;
    tenpack_format_t format;
};

PYBIND11_MODULE(tenpack_module, t) {
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
                 tenpack_guess_format(data.data(), data.size(), &self.format, &self.ctx);
                 return true;
             })
        .def("guess_dims",
             [](py_tenpack_t& self, std::vector<uint8_t> data) {
                 tenpack_guess_dimensions(data.data(), data.size(), self.format, &self.dims, &self.ctx);
                 return true;
             })
        .def("unpack",
             [](py_tenpack_t& self, std::vector<uint8_t> data, std::vector<uint8_t> output) {
                 output.resize(self.dims.bytes_per_channel * self.dims.channels * self.dims.frames * self.dims.height *
                               self.dims.width);
                 tenpack_unpack(data.data(), data.size(), self.format, &self.dims, output.data(), &self.ctx);
                 return true;
             })
        .def("ctx_free", [](py_tenpack_t& self) {
            tenpack_context_free(self.ctx);
            return true;
        });
}
