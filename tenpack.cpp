#include <array>
#include <cinttypes>

#include <turbojpeg.h>
#include <spng.h>
#include <decoder.h>

#include "tenpack.h"

constexpr unsigned char prefix_jpeg_k[3] {0xFF, 0xD8, 0xFF};
constexpr unsigned char prefix_png_k[4] {0x89, 0x50, 0x4E, 0x47};
constexpr unsigned char prefix_gif_k[3] {0x47, 0x49, 0x46};
constexpr unsigned char prefix_bmp_k[2] {0x42, 0x4D};
constexpr unsigned char prefix_jpeg2000_k[13] {0x0, 0x0, 0x0, 0xC, 0x6A, 0x50, 0x20, 0x20, 0xD, 0xA, 0x87, 0xA, 0x0};
constexpr unsigned char prefix_jxr_k[3] {0x49, 0x49, 0xBC};
constexpr unsigned char prefix_psd_k[4] {0x38, 0x42, 0x50, 0x53};
constexpr unsigned char prefix_ico_k[4] {0x00, 0x00, 0x01, 0x00};
constexpr unsigned char prefix_dwg_k[4] {0x41, 0x43, 0x31, 0x30};

template <typename at, std::size_t length_ak>
bool matches(at (&prefix)[length_ak], at* content, std::size_t content_len) {
    if (content_len < length_ak) [[unlikely]]
        return false;
    bool matches = true;
#pragma unroll_completely
    for (std::size_t i = 0; i != length_ak; ++i)
        matches &= prefix[i] == content[i];
    return matches;
}

bool tenpack_guess_format( //
    void* content_bytes,
    size_t content_length,
    tenpack_format_t* format) {

    auto content = reinterpret_cast<unsigned char const*>(content_bytes);

    // clang-format off
    if (*format = tenpack_jpeg_k; matches(prefix_jpeg_k, content, content_length)) return true;
    if (*format = tenpack_png_k; matches(prefix_png_k, content, content_length)) return true;
    if (*format = tenpack_gif_k; matches(prefix_gif_k, content, content_length)) return true;
    if (*format = tenpack_bmp_k; matches(prefix_bmp_k, content, content_length)) return true;
    if (*format = tenpack_jpeg2000_k; matches(prefix_jpeg2000_k, content, content_length)) return true;
    if (*format = tenpack_jxr_k; matches(prefix_jxr_k, content, content_length)) return true;
    if (*format = tenpack_psd_k; matches(prefix_psd_k, content, content_length)) return true;
    if (*format = tenpack_ico_k; matches(prefix_ico_k, content, content_length)) return true;
    if (*format = tenpack_dwg_k; matches(prefix_dwg_k, content, content_length)) return true;
    // clang-format on

    return false;
}

bool tenpack_guess_dimensions( //
    void* content_bytes,
    size_t content_length,
    tenpack_format_t format,
    size_t* guessed_dimensions) {

    // There is some documentation in libjpeg.txt. libjpeg is a very low-level, steep-learning-curve,
    // old school c library. To use it effectively you need to be familiar with setjmp and longjmp,
    // c structure layouts, function pointers, and lots of other low-level C stuff. It's a bear to
    // work with but possible to do a great deal with minimal resource usage.

    // Docs: https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html

    switch (format) {
    case tenpack_format_t::tenpack_jpeg_k: {
        int jpeg_width, jpeg_height, jpeg_sub_sample, jpeg_color_space;
        tjhandle handle = tjInitDecompress();
        bool success = tjDecompressHeader3(handle,
                                           reinterpret_cast<unsigned char const*>(content_bytes),
                                           static_cast<unsigned long>(content_length),
                                           &jpeg_width,
                                           &jpeg_height,
                                           &jpeg_sub_sample,
                                           &jpeg_color_space) == 0;

        guessed_dimensions[0] = static_cast<size_t>(jpeg_width);
        guessed_dimensions[1] = static_cast<size_t>(jpeg_height);
        guessed_dimensions[2] = 4;

        return success;
    }

    case tenpack_format_t::tenpack_png_k: {
        spng_ihdr ihdr;
        size_t out_size;
        spng_ctx* ctx = spng_ctx_new(0);
        spng_set_png_buffer(ctx, content_bytes, content_length);
        bool success = spng_get_ihdr(ctx, &ihdr) == 0;
        spng_ctx_free(ctx);

        guessed_dimensions[0] = static_cast<size_t>(ihdr.width);
        guessed_dimensions[1] = static_cast<size_t>(ihdr.height);
        guessed_dimensions[2] = 4;

        return success;
    }

    default: return false;
    }
}

bool tenpack_upack( //
    void* content_bytes,
    size_t content_length,
    tenpack_format_t format,
    size_t* slice,
    void* output_begin,
    size_t output_stride) {

    switch (format) {
    case tenpack_format_t::tenpack_jpeg_k: {

        // Pixel formats:
        // https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html#gac916144e26c3817ac514e64ae5d12e2a
        // Flags:
        // https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html#gacb233cfd722d66d1ccbf48a7de81f0e0
        tjhandle handle = tjInitDecompress();
        bool success = tjDecompress2(handle,
                                     reinterpret_cast<unsigned char const*>(content_bytes),
                                     static_cast<unsigned long>(content_length),
                                     reinterpret_cast<unsigned char*>(output_begin),
                                     0,
                                     output_stride,
                                     0,
                                     TJPF_RGBA,
                                     0);
        return success;
    }
    case tenpack_format_t::tenpack_png_k: {

        bool success;

        // Pixel formats:
        // https://libspng.org/docs/context/#spng_format
        //  Flags:
        //  https://libspng.org/docs/decode/#spng_decode_flags
        spng_ctx* ctx = spng_ctx_new(0);
        spng_set_png_buffer(ctx, content_bytes, content_length);
        spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &output_stride);
        spng_decode_image(ctx, output_begin, output_stride, SPNG_FMT_RGBA8, 0);
        spng_ctx_free(ctx);

        return success;
    }

    default: return false;
    }
}
