#include <fcntl.h>    // `open` files
#include <sys/stat.h> // `stat` to obtain file metadata
#include <sys/mman.h> // `mmap` to read datasets faster
#include <unistd.h>   // `close` files

#include <filesystem>
#include <thread>
#include <exception>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/cast.h>

#include "tenpack.h"

namespace py = pybind11;

struct py_context_t {
    tenpack_ctx_t native_ = nullptr;

    py_context_t() = default;
    py_context_t(py_context_t&& other) noexcept{ std::swap(native_, other.native_); }
    py_context_t& operator=(py_context_t&& other) noexcept{
        std::swap(native_, other.native_);
        return *this;
    }
    py_context_t(py_context_t const&) = delete;
    py_context_t& operator=(py_context_t const&) = delete;
    ~py_context_t() noexcept { tenpack_context_free(native_); }
};

struct py_pack_t {
    tenpack_format_t format = tenpack_format_unknown_k;
    tenpack_shape_t shape = {0u, 0u, 0u, 0u, 0u, false};
    py::object tensor = py::none();
    void* data = nullptr;
};

enum class media_kind_t {
    image_k,
    animation_k,
    audio_k,
    video_k,
    other_k,
};

static std::vector<py_context_t> contexts;

media_kind_t media_kind(tenpack_format_t fmt) {
    switch (fmt) {
    case tenpack_bmp_k: return media_kind_t::image_k;
    case tenpack_jxr_k: return media_kind_t::image_k;
    case tenpack_png_k: return media_kind_t::image_k;
    case tenpack_ico_k: return media_kind_t::image_k;
    case tenpack_jpeg_k: return media_kind_t::image_k;
    case tenpack_jpeg2000_k: return media_kind_t::image_k;
    case tenpack_gif_k: return media_kind_t::animation_k;
    case tenpack_wav_k: return media_kind_t::audio_k;
    case tenpack_avi_k: return media_kind_t::video_k;
    case tenpack_mpeg4_k: return media_kind_t::video_k;
    default: return media_kind_t::other_k;
    }
}

template <typename scalar_at>
py_pack_t py_pack(std::initializer_list<std::size_t> shape) {
    auto tensor = py::array_t<scalar_at>(shape);
    py_pack_t result;
    result.data = tensor.request().ptr;
    result.tensor = std::move(tensor);
    return result;
}

py_pack_t py_pack(tenpack_shape_t const& shape, media_kind_t kind) {

    switch (kind) {
    case media_kind_t::image_k: {
        if (!shape.is_signed)
            switch (shape.bytes_per_channel) {
            case 1: return py_pack<uint8_t>({shape.height, shape.width, shape.channels});
            case 2: return py_pack<uint16_t>({shape.height, shape.width, shape.channels});
            case 4: return py_pack<uint32_t>({shape.height, shape.width, shape.channels});
            case 8: return py_pack<uint64_t>({shape.height, shape.width, shape.channels});
            default: return {};
            }
        else
            switch (shape.bytes_per_channel) {
            case 1: return py_pack<int8_t>({shape.height, shape.width, shape.channels});
            case 2: return py_pack<int16_t>({shape.height, shape.width, shape.channels});
            case 4: return py_pack<int32_t>({shape.height, shape.width, shape.channels});
            case 8: return py_pack<int64_t>({shape.height, shape.width, shape.channels});
            default: return {};
            }
    }
    case media_kind_t::animation_k: {
        if (!shape.is_signed)
            switch (shape.bytes_per_channel) {
            case 1: return py_pack<uint8_t>({shape.frames, shape.height, shape.width, shape.channels});
            case 2: return py_pack<uint16_t>({shape.frames, shape.height, shape.width, shape.channels});
            case 4: return py_pack<uint32_t>({shape.frames, shape.height, shape.width, shape.channels});
            case 8: return py_pack<uint64_t>({shape.frames, shape.height, shape.width, shape.channels});
            default: return {};
            }
        else
            switch (shape.bytes_per_channel) {
            case 1: return py_pack<int8_t>({shape.frames, shape.height, shape.width, shape.channels});
            case 2: return py_pack<int16_t>({shape.frames, shape.height, shape.width, shape.channels});
            case 4: return py_pack<int32_t>({shape.frames, shape.height, shape.width, shape.channels});
            case 8: return py_pack<int64_t>({shape.frames, shape.height, shape.width, shape.channels});
            default: return {};
            }
    }
    case media_kind_t::audio_k: {
        switch (shape.bytes_per_channel) {
        case 1: return py_pack<int8_t>({shape.width});
        case 2: return py_pack<int16_t>({shape.width});
        case 4: return py_pack<int32_t>({shape.width});
        case 8: return py_pack<int64_t>({shape.width});
        default: return {};
        }
    }
    default: return {};
    }
}

static tenpack_format_t api_format(std::string_view data) {

    
    if (contexts.empty())
        contexts.emplace_back(py_context_t {});

    tenpack_ctx_t& ctx = contexts.back().native_;
    tenpack_format_t format = tenpack_format_unknown_k;
    if (!tenpack_guess_format(data.data(), data.size(), &format, &ctx))
        throw std::invalid_argument("Couldn't guess format!");

    return format;
}

static tenpack_shape_t api_shape(std::string_view data) {

    
    if (contexts.empty())
        contexts.emplace_back(py_context_t {});

    tenpack_ctx_t& ctx = contexts.back().native_;
    tenpack_format_t format = tenpack_format_unknown_k;
    if (!tenpack_guess_format(data.data(), data.size(), &format, &ctx))
        throw std::invalid_argument("Couldn't guess format!");

    tenpack_shape_t shape {};
    if (!tenpack_guess_shape(data.data(), data.size(), format, &shape, &ctx))
        throw std::invalid_argument("Couldn't guess extract shape!");

    return shape;
}

static py_pack_t api_unpack(std::string_view data) {

    
    if (contexts.empty())
        contexts.emplace_back(py_context_t {});

    tenpack_ctx_t& ctx = contexts.back().native_;
    tenpack_format_t format = tenpack_format_unknown_k;
    if (!tenpack_guess_format(data.data(), data.size(), &format, &ctx))
        throw std::invalid_argument("Couldn't guess format!");

    tenpack_shape_t shape {};
    if (!tenpack_guess_shape(data.data(), data.size(), format, &shape, &ctx))
        throw std::invalid_argument("Couldn't guess extract shape!");

    py_pack_t pack = py_pack(shape, media_kind(format));
    tenpack_unpack(data.data(), data.size(), format, &shape, pack.data, &ctx);
    pack.format = format;
    pack.shape = shape;
    return pack;
}

static py::tuple api_unpack_paths(std::vector<std::string> const& paths, std::size_t threads_count) {
    std::size_t size = paths.size();
    py::tuple results(size);
    py::gil_scoped_release release;

    if (!threads_count) {
        std::size_t min_images_per_thread = 10;
        threads_count = size + (min_images_per_thread - 1) / min_images_per_thread;
    }

    // Prepare contexts
    while (contexts.size() < threads_count)
        contexts.emplace_back(py_context_t {});

    std::size_t batch_size = size / threads_count;
    auto job = [&](std::size_t thread_idx) {
        std::size_t batch_start = batch_size * thread_idx;
        for (std::size_t image_idx = batch_start; image_idx != batch_start + batch_size; ++image_idx) {
            std::string const& path = paths[image_idx];

            // Obtain descriptor
            int descriptor = open(path.c_str(), O_RDONLY);
            if (descriptor < 0)
                throw std::runtime_error(std::strerror(errno));

            // Estimate the file size
            struct stat file_stat;
            int fstat_status = fstat(descriptor, &file_stat);
            if (fstat_status < 0) {
                ::close(descriptor);
                throw std::runtime_error(std::strerror(errno));
            }

            // Map the entire file
            char const* file = (char const*)mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, descriptor, 0);
            if (file == MAP_FAILED) {
                ::close(descriptor);
                throw std::runtime_error(std::strerror(errno));
            }
            auto release_mapping = [&] {
                munmap((void*)file, file_stat.st_size);
                ::close(descriptor);
            };

            std::string_view data {file, static_cast<std::size_t>(file_stat.st_size)};
            tenpack_ctx_t& ctx = contexts[thread_idx].native_;
            tenpack_format_t format = tenpack_format_unknown_k;
            if (!tenpack_guess_format(data.data(), data.size(), &format, &ctx)) {
                release_mapping();
                throw std::invalid_argument("Couldn't guess format!");
            }

            tenpack_shape_t shape {};
            if (!tenpack_guess_shape(data.data(), data.size(), format, &shape, &ctx)) {
                release_mapping();
                throw std::invalid_argument("Couldn't guess extract shape!");
            }

            py::gil_scoped_acquire acquire;
            py_pack_t pack = py_pack(shape, media_kind(format));
            py::gil_scoped_release release;

            tenpack_unpack(data.data(), data.size(), format, &shape, pack.data, &ctx);
            pack.format = format;
            pack.shape = shape;
            release_mapping();

            py::gil_scoped_acquire acquire2;
            results[image_idx] = pack;
            py::gil_scoped_release release2;
        }
    };

    std::vector<std::thread> threads;
    for (std::size_t thread_idx = 1; thread_idx != threads_count; ++thread_idx)
        threads.emplace_back([&, thread_idx = thread_idx]() { job(thread_idx); });
    job(0);
    for (std::thread& thread : threads)
        thread.join();

    py::gil_scoped_acquire acquire;
    return results;
}

PYBIND11_MODULE(tenpack, m) {
    m.doc() =
        "Python bindings for TenPack multimedia unpacking library.\n"
        "Supports:\n"
        "> Guess file format from MetaData.\n"
        "> Guess file property.\n"
        "> Get unpacked file in tensor.\n"
        "---------------------------------------------\n";

    py::enum_<tenpack_format_t>(m, "Format")
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

    py::class_<tenpack_shape_t>(m, "Shape")
        .def_readonly("frames", &tenpack_shape_t::frames)
        .def_readonly("width", &tenpack_shape_t::width)
        .def_readonly("height", &tenpack_shape_t::height)
        .def_readonly("channels", &tenpack_shape_t::channels)
        .def_readonly("bytes_per_channel", &tenpack_shape_t::bytes_per_channel)
        .def_readonly("is_signed", &tenpack_shape_t::is_signed);

    py::class_<py_pack_t>(m, "Pack")
        .def_readonly("format", &py_pack_t::format)
        .def_readonly("shape", &py_pack_t::shape)
        .def_readonly("tensor", &py_pack_t::tensor);

    m.def("format", &api_format);
    m.def("shape", &api_shape);
    m.def("unpack", &api_unpack);
    m.def("unpack_paths", &api_unpack_paths, py::arg("paths"), py::arg("threads") = 0);
}
