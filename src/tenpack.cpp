#include <memory>
#include <cstring>
#include <cinttypes>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <turbojpeg.h>
#include <spng.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <nsgif.h>
}

#include "tenpack.h"

constexpr size_t bytes_per_pixel_k = 4ull;
constexpr size_t max_image_bytes_k = 48ull * 1024ull * 1024ull;

// Image
constexpr uint8_t prefix_jpeg_k[3] {0xFF, 0xD8, 0xFF};
constexpr uint8_t prefix_png_k[4] {0x89, 0x50, 0x4E, 0x47};
constexpr uint8_t prefix_gif_k[3] {0x47, 0x49, 0x46};
constexpr uint8_t prefix_bmp_k[2] {0x42, 0x4D};
constexpr uint8_t prefix_jpeg2000_k[13] {0x0, 0x0, 0x0, 0xC, 0x6A, 0x50, 0x20, 0x20, 0xD, 0xA, 0x87, 0xA, 0x0};
constexpr uint8_t prefix_jxr_k[3] {0x49, 0x49, 0xBC};
constexpr uint8_t prefix_psd_k[4] {0x38, 0x42, 0x50, 0x53};
constexpr uint8_t prefix_ico_k[4] {0x00, 0x00, 0x01, 0x00};
constexpr uint8_t prefix_dwg_k[4] {0x41, 0x43, 0x31, 0x30};

// The Resource Interchange File Format (RIFF)
// is a generic file container format for storing
// data in tagged chunks.
// About RIFF -> https://en.wikipedia.org/wiki/Resource_Interchange_File_Format
constexpr uint8_t prefix_riff_k[4] {0x52, 0x49, 0x46, 0x46};

// Audio
constexpr uint8_t prefix_wav_k[4] {0x57, 0x41, 0x56, 0x45};

// Video
constexpr uint8_t prefix_mpeg4_k[8] {0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D};

static void* bitmap_create(int width, int height) {
    /* ensure a stupidly large bitmap is not created */
    if (((long long)width * (long long)height) > (max_image_bytes_k / bytes_per_pixel_k)) {
        return NULL;
    }
    return calloc(width * height, bytes_per_pixel_k);
}

static void bitmap_set_opaque(void* bitmap, bool opaque) {
}

static bool bitmap_test_opaque(void* bitmap) {
    return false;
}

static uint8_t* bitmap_get_buffer(void* bitmap) {
    assert(bitmap);
    return reinterpret_cast<uint8_t*>(bitmap);
}

static void bitmap_destroy(void* bitmap) {
    assert(bitmap);
    free(bitmap);
}

static void bitmap_modified(void* bitmap) {
}

template <typename at, size_t length_ak>
bool matches(at (&prefix)[length_ak], at* content, size_t content_len) {
    if (content_len < length_ak) [[unlikely]]
        return false;
    bool matches = true;
#pragma unroll_completely
    for (size_t i = 0; i != length_ak; ++i)
        matches &= prefix[i] == content[i];
    return matches;
}

static nsgif_bitmap_cb_vt bitmap_callbacks {
    bitmap_create,
    bitmap_destroy,
    bitmap_get_buffer,
    bitmap_set_opaque,
    bitmap_test_opaque,
    bitmap_modified,
};

class ctx_t {

    tjhandle jpeg_ = nullptr;
    spng_ctx* png_ = nullptr;
    nsgif_t* gif_ = nullptr;
    // ma_decoder wav_;
    drwav wav_;
    bool wav_state = false;

  public:
    ~ctx_t() {
        if (jpeg_)
            tjDestroy(jpeg_);
        if (png_)
            spng_ctx_free(png_);
        if (gif_)
            nsgif_destroy(gif_);
        if (wav_state = false; wav_state)
            drwav_uninit(&wav_);
    }

    spng_ctx* png() { return png_ = png_ ?: spng_ctx_new(0); }
    tjhandle jpeg() { return jpeg_ = jpeg_ ?: tjInitDecompress(); }
    nsgif_t* gif() {
        if (!gif_)
            nsgif_create(&bitmap_callbacks, NSGIF_BITMAP_FMT_R8G8B8A8, &gif_);
        return gif_;
    };
    drwav* wav() {
        if (!wav_state)
            wav_state = true;
        return &wav_;
    }
};

size_t size_bytes(tenpack_dimensions_t const& dims, tenpack_format_t const format) {
    if (format == tenpack_wav_k)
        return dims.frames * dims.channels * dims.width;
    return dims.frames * dims.channels * dims.width * dims.height * dims.bytes_per_channel;
}

bool tenpack_guess_format( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t* format,
    tenpack_ctx_t* context) {

    auto content = reinterpret_cast<uint8_t const*>(data);

    // clang-format off
    if (*format = tenpack_jpeg_k; matches(prefix_jpeg_k, content, len)) return true;
    if (*format = tenpack_png_k; matches(prefix_png_k, content, len)) return true;
    if (*format = tenpack_gif_k; matches(prefix_gif_k, content, len)) return true;
    if (*format = tenpack_bmp_k; matches(prefix_bmp_k, content, len)) return true;
    if (*format = tenpack_jpeg2000_k; matches(prefix_jpeg2000_k, content, len)) return true;
    if (*format = tenpack_jxr_k; matches(prefix_jxr_k, content, len)) return true;
    if (*format = tenpack_psd_k; matches(prefix_psd_k, content, len)) return true;
    if (*format = tenpack_ico_k; matches(prefix_ico_k, content, len)) return true;
    if (*format = tenpack_dwg_k; matches(prefix_dwg_k, content, len)) return true;

    if (matches(prefix_riff_k, content, len))
    {
        if (matches(prefix_wav_k, content + 8, len - 8))
        {
            *format = tenpack_wav_k; 
             return true;
        }
    }

    // MPEG4 format offset is 4  
    if (*format = tenpack_mpeg4_k; matches(prefix_mpeg4_k, content + 4, len - 4)) return true;

    // clang-format on

    return false;
}

bool tenpack_guess_dimensions( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t const format,
    tenpack_dimensions_t* dimensions,
    tenpack_ctx_t* context) {

    tenpack_dimensions_t& dims = *dimensions;
    ctx_t* ctx_ptr = *reinterpret_cast<ctx_t**>(context);
    if (!ctx_ptr)
        ctx_ptr = new ctx_t();
    if (!ctx_ptr)
        return false;

    switch (format) {

    case tenpack_format_t::tenpack_jpeg_k: {
        // There is some documentation in libjpeg.txt. libjpeg is a very low-level, steep-learning-curve,
        // old school c library. To use it effectively you need to be familiar with setjmp and longjmp,
        // c structure layouts, function pointers, and lots of other low-level C stuff. It's a bear to
        // work with but possible to do a great deal with minimal resource usage.
        // Docs: https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html
        int jpeg_width = 0, jpeg_height = 0, jpeg_sub_sample = 0, jpeg_color_space = 0;
        bool success = tjDecompressHeader3( //
                           ctx_ptr->jpeg(),
                           reinterpret_cast<uint8_t const*>(data),
                           static_cast<unsigned long>(len),
                           &jpeg_width,
                           &jpeg_height,
                           &jpeg_sub_sample,
                           &jpeg_color_space) == 0;
        TJCS color_space = static_cast<TJCS>(jpeg_color_space);
        switch (color_space) {
        case TJCS_GRAY: dims.channels = 1; break;
        case TJCS_YCbCr: dims.channels = 3; break;
        case TJCS_RGB: dims.channels = 3; break;
        default: dims.channels = 3; break;
        // ? Unsupported:
        case TJCS_CMYK: dims.channels = 4; break;
        case TJCS_YCCK: dims.channels = 4; break;
        }

        dims.width = static_cast<size_t>(jpeg_width);
        dims.height = static_cast<size_t>(jpeg_height);
        dims.bytes_per_channel = 1;
        dims.frames = 1;
        return success;
    }

    case tenpack_format_t::tenpack_png_k: {
        // Image header: https://libspng.org/docs/chunk/#spng_get_ihdr
        spng_ihdr ihdr;
        spng_set_png_buffer(ctx_ptr->png(), data, len);
        bool success = spng_get_ihdr(ctx_ptr->png(), &ihdr) == 0;

        dims.width = static_cast<size_t>(ihdr.width);
        dims.height = static_cast<size_t>(ihdr.height);
        dims.bytes_per_channel = ihdr.bit_depth / 8;
        dims.frames = 1;

        switch (ihdr.color_type) {
        case SPNG_COLOR_TYPE_GRAYSCALE: dims.channels = 1; break;
        case SPNG_COLOR_TYPE_INDEXED: dims.channels = 1; break;
        case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA: dims.channels = 2; break;
        case SPNG_COLOR_TYPE_TRUECOLOR: dims.channels = 3; break;
        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA: dims.channels = 4; break;
        default: return false;
        }

        return success;
    }

    case tenpack_format_t::tenpack_gif_k: {

        bool success = nsgif_data_scan(ctx_ptr->gif(), len, (uint8_t*)data) == 0;

        auto info = nsgif_get_info(ctx_ptr->gif());

        dims.bytes_per_channel = 1;
        dims.channels = 3;
        dims.width = info->width;
        dims.height = info->height;
        dims.frames = info->frame_count;

        return success;
    }

    case tenpack_format_t::tenpack_wav_k: {

        bool success = drwav_init_memory(ctx_ptr->wav(), data, len, NULL);

        dims.bytes_per_channel = ctx_ptr->wav()->bitsPerSample / 8;
        dims.channels = ctx_ptr->wav()->channels;
        dims.frames = ctx_ptr->wav()->totalPCMFrameCount;
        dims.height = ctx_ptr->wav()->sampleRate;
        dims.width = dims.frames * dims.channels;
        dims.is_signed = !ctx_ptr->wav()->aiff.isUnsigned;
        return success;
    }

    default: return false;
    }
}

bool tenpack_unpack( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t const format,
    tenpack_dimensions_t const* output_dimensions,
    void* output,
    tenpack_ctx_t* context) {

    tenpack_dimensions_t const& dims = *output_dimensions;
    ctx_t* ctx_ptr = *reinterpret_cast<ctx_t**>(context);
    if (!ctx_ptr)
        ctx_ptr = new ctx_t();
    if (!ctx_ptr)
        return false;

    switch (format) {

    case tenpack_format_t::tenpack_jpeg_k: {
        // Decoding API:
        // https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html#gae9eccef8b682a48f43a9117c231ed013
        // Pixel formats:
        // https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html#gac916144e26c3817ac514e64ae5d12e2a
        // Flags:
        // https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html#gacb233cfd722d66d1ccbf48a7de81f0e0
        TJPF pixel_format;
        if (dims.bytes_per_channel == 1 && dims.channels == 3)
            pixel_format = TJPF_RGB;
        else if (dims.bytes_per_channel == 1 && dims.channels == 1)
            pixel_format = TJPF_GRAY;
        else if (dims.bytes_per_channel == 1 && dims.channels == 4)
            pixel_format = TJPF_RGBA;
        else
            return false;

        bool success = tjDecompress2( //
                           ctx_ptr->jpeg(),
                           reinterpret_cast<uint8_t const*>(data),
                           static_cast<unsigned long>(len),
                           reinterpret_cast<uint8_t*>(output),
                           output_dimensions->width,  // width
                           0,                         // pitch
                           output_dimensions->height, // height
                           pixel_format,
                           0) == 0;
        return success;
    }

    case tenpack_format_t::tenpack_png_k: {
        // Decoding API: https://libspng.org/docs/decode/#spng_decode_image
        // Pixel formats: https://libspng.org/docs/context/#spng_format
        // Flags: https://libspng.org/docs/decode/#spng_decode_flags
        // TODO: What "This function can only be called once per context" means?!

        spng_ihdr ihdr;
        auto spng_context = ctx_ptr->png();
        spng_set_png_buffer(spng_context, data, len);
        spng_get_ihdr(spng_context, &ihdr);
        spng_format fmt;

        if (dims.bytes_per_channel == 1 && dims.channels == 4)
            fmt = SPNG_FMT_RGBA8;
        else if (dims.bytes_per_channel == 1 && dims.channels == 3)
            fmt = SPNG_FMT_RGB8;
        else if (dims.bytes_per_channel == 2 && dims.channels == 2)
            fmt = SPNG_FMT_GA16;
        else if (dims.bytes_per_channel == 1 && dims.channels == 2)
            fmt = SPNG_FMT_GA8;
        else if (dims.bytes_per_channel == 1 && dims.channels == 1)
            fmt = SPNG_FMT_G8;
        else if (dims.bytes_per_channel == 2 && dims.channels == 4)
            fmt = SPNG_FMT_RGBA16;
        else
            return false;

        bool success = spng_decode_image(spng_context, output, size_bytes(dims, format), fmt, 0) == 0;
        return success;
    }

    case tenpack_format_t::tenpack_gif_k: {

        bool success = nsgif_data_scan(ctx_ptr->gif(), len, (uint8_t*)data) == 0;
        if (!success)
            return false;

        size_t const size = dims.channels * dims.height * dims.width;
        for (size_t frame_num = 0; frame_num < dims.frames; ++frame_num) {
            void* bitmap = nullptr;
            success = nsgif_frame_decode(ctx_ptr->gif(), frame_num, &bitmap) == 0;
            if (!success)
                return false;
            memcpy(reinterpret_cast<uint8_t*>(output) + size * frame_num, bitmap, size);
        }

        return success;
    }

    case tenpack_format_t::tenpack_wav_k: {

        bool success = drwav_init_memory(ctx_ptr->wav(), data, len, NULL);
        if (!success)
            return false;
        return drwav_read_pcm_frames(ctx_ptr->wav(), dims.width, output) == dims.width;
    }

    default: return false;
    }
}

bool tenpack_context_free(tenpack_ctx_t ctx) {
    if (ctx) {
        delete reinterpret_cast<ctx_t*>(ctx);
        return true;
    }
    else
        return false;
}