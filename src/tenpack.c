#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <turbojpeg.h>
#include <spng.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "tenpack.h"

// Image
static uint8_t const prefix_jpeg_k[] = {0xFF, 0xD8, 0xFF};
static uint8_t const prefix_png_k[] = {0x89, 0x50, 0x4E, 0x47};
static uint8_t const prefix_bmp_k[] = {0x42, 0x4D};
static uint8_t const prefix_jpeg2000_k[] = {0x0, 0x0, 0x0, 0xC, 0x6A, 0x50, 0x20, 0xD, 0xA, 0x87, 0xA, 0x0};
static uint8_t const prefix_jxr_k[] = {0x49, 0x49, 0xBC};
static uint8_t const prefix_ico_k[] = {0x00, 0x00, 0x01, 0x00};

// Animation
static uint8_t const prefix_gif_k[] = {0x47, 0x49, 0x46};

// RIFF
static uint8_t const prefix_riff_k[] = {0x52, 0x49, 0x46, 0x46};
static uint8_t const prefix_wav_k[] = {0x57, 0x41, 0x56, 0x45};

// Video
static uint8_t const prefix_mpeg4_k[] = {0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D};

// Other
static uint8_t const prefix_dwg_k[] = {0x41, 0x43, 0x31, 0x30};
static uint8_t const prefix_psd_k[] = {0x38, 0x42, 0x50, 0x53};

typedef struct {
    tjhandle jpeg_;
    spng_ctx* png_;
    drwav wav_;
    int wav_state;
} ctx_t;

// Utility functions
ctx_t* new_ctx() {
    ctx_t* ctx = (ctx_t*)malloc(sizeof(ctx_t));
    if (!ctx)
        return NULL;
    ctx->jpeg_ = NULL;
    ctx->png_ = NULL;
    ctx->wav_state = 0;
    return ctx;
}

spng_ctx* get_png_ctx(ctx_t* ctx) {
    if (!ctx->png_)
        ctx->png_ = spng_ctx_new(0);
    return ctx->png_;
}

tjhandle get_jpeg_ctx(ctx_t* ctx) {
    if (!ctx->jpeg_)
        ctx->jpeg_ = tjInitDecompress();
    return ctx->jpeg_;
}

drwav* get_wav_ctx(ctx_t* ctx) {
    if (!ctx->wav_state)
        ctx->wav_state = 1;
    return &(ctx->wav_);
}

bool matches(uint8_t const* prefix, size_t prefix_len, uint8_t const* content, size_t content_len) {
    if (content_len < prefix_len)
        return 0;
    for (size_t i = 0; i < prefix_len; ++i) {
        if (prefix[i] != content[i])
            return false;
    }
    return true;
}

size_t size_bytes(tenpack_shape_t const* shape, tenpack_format_t format) {
    if (format == tenpack_wav_k)
        return shape->frames * shape->width;
    return shape->frames * shape->channels * shape->width * shape->height * shape->bytes_per_channel;
}

// Core functionality
bool tenpack_guess_format( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t* format,
    tenpack_ctx_t* context) {

    uint8_t const* content = (uint8_t const*)data;

    if ((*format = tenpack_jpeg_k, matches(prefix_jpeg_k, sizeof(prefix_jpeg_k), content, len)))
        return true;
    if ((*format = tenpack_png_k, matches(prefix_png_k, sizeof(prefix_png_k), content, len)))
        return true;
    if ((*format = tenpack_gif_k, matches(prefix_gif_k, sizeof(prefix_gif_k), content, len)))
        return true;
    if ((*format = tenpack_bmp_k, matches(prefix_bmp_k, sizeof(prefix_bmp_k), content, len)))
        return true;
    if ((*format = tenpack_jpeg2000_k, matches(prefix_jpeg2000_k, sizeof(prefix_jpeg2000_k), content, len)))
        return true;
    if ((*format = tenpack_jxr_k, matches(prefix_jxr_k, sizeof(prefix_jxr_k), content, len)))
        return true;
    if ((*format = tenpack_psd_k, matches(prefix_psd_k, sizeof(prefix_psd_k), content, len)))
        return true;
    if ((*format = tenpack_ico_k, matches(prefix_ico_k, sizeof(prefix_ico_k), content, len)))
        return true;
    if ((*format = tenpack_dwg_k, matches(prefix_dwg_k, sizeof(prefix_dwg_k), content, len)))
        return true;
    if (matches(prefix_riff_k, sizeof(prefix_riff_k), content, len) &&
        matches(prefix_wav_k, sizeof(prefix_wav_k), content + 8, len - 8)) {
        *format = tenpack_wav_k;
        return true;
    }
    if ((*format = tenpack_mpeg4_k, matches(prefix_mpeg4_k, sizeof(prefix_mpeg4_k), content + 4, len - 4)))
        return true;

    return false;
}

bool tenpack_guess_shape( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t format,
    tenpack_shape_t* dimensions,
    tenpack_ctx_t* context) {

    tenpack_shape_t* shape = dimensions;
    ctx_t* ctx_ptr = (ctx_t*)(*context);
    if (!ctx_ptr) {
        ctx_ptr = new_ctx();
        if (!ctx_ptr)
            return 0;
    }

    switch (format) {
    case tenpack_jpeg_k: {
        int jpeg_width = 0, jpeg_height = 0, jpeg_sub_sample = 0, jpeg_color_space = 0;
        int success = tjDecompressHeader3( //
                          get_jpeg_ctx(ctx_ptr),
                          (uint8_t*)data,
                          (unsigned long)len,
                          &jpeg_width,
                          &jpeg_height,
                          &jpeg_sub_sample,
                          &jpeg_color_space) == 0;

        switch (jpeg_color_space) {
        case TJCS_GRAY: shape->channels = 1; break;
        case TJCS_YCbCr: shape->channels = 3; break;
        case TJCS_RGB: shape->channels = 3; break;
        default: shape->channels = 3; break;
        }

        shape->width = (size_t)jpeg_width;
        shape->height = (size_t)jpeg_height;
        shape->bytes_per_channel = 1;
        shape->frames = 1;
        return success;
    }

    case tenpack_png_k: {
        struct spng_ihdr image_header;
        spng_ctx* spng_context = get_png_ctx(ctx_ptr);
        spng_set_png_buffer(spng_context, data, len);
        int success = spng_get_ihdr(spng_context, &image_header) == 0;

        shape->width = (size_t)image_header.width;
        shape->height = (size_t)image_header.height;
        shape->bytes_per_channel = image_header.bit_depth / 8;
        shape->frames = 1;

        switch (image_header.color_type) {
        case SPNG_COLOR_TYPE_GRAYSCALE: shape->channels = 1; break;
        case SPNG_COLOR_TYPE_INDEXED: shape->channels = 1; break;
        case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA: shape->channels = 2; break;
        case SPNG_COLOR_TYPE_TRUECOLOR: shape->channels = 3; break;
        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA: shape->channels = 4; break;
        default: return false;
        }

        return success;
    }

    case tenpack_gif_k: {
        int* delays = NULL;
        int comp = 0, req_comp = 3;
        int width = 0, height = 0, frames = 0;
        uint8_t* result = stbi_load_gif_from_memory( //
            (uint8_t*)data,
            len,
            &delays,
            &width,
            &height,
            &frames,
            &comp,
            req_comp);

        shape->bytes_per_channel = 1;
        shape->channels = 3;
        shape->width = (size_t)width;
        shape->height = (size_t)height;
        shape->frames = (size_t)frames;

        return result != NULL;
    }

    case tenpack_wav_k: {
        int success = drwav_init_memory(get_wav_ctx(ctx_ptr), data, len, NULL);

        shape->bytes_per_channel = ctx_ptr->wav_.bitsPerSample == 12 ? 2 : ctx_ptr->wav_.bitsPerSample / 8;
        shape->channels = ctx_ptr->wav_.channels;
        shape->frames = ctx_ptr->wav_.totalPCMFrameCount;
        shape->height = ctx_ptr->wav_.sampleRate;
        shape->width = shape->frames * shape->channels;
        shape->is_signed = !ctx_ptr->wav_.aiff.isUnsigned;
        return success;
    }

    default: return false;
    }
}

bool tenpack_unpack( //
    tenpack_input_t const data,
    size_t len,
    tenpack_format_t format,
    tenpack_shape_t const* output_shape,
    void* output,
    tenpack_ctx_t* context) {

    tenpack_shape_t const* shape = output_shape;
    ctx_t* ctx_ptr = (ctx_t*)(*context);
    if (!ctx_ptr) {
        ctx_ptr = new_ctx();
        if (!ctx_ptr)
            return false;
    }

    switch (format) {
    case tenpack_jpeg_k: {
        enum TJPF pixel_format;
        if (shape->bytes_per_channel == 1 && shape->channels == 3)
            pixel_format = TJPF_RGB;
        else if (shape->bytes_per_channel == 1 && shape->channels == 1)
            pixel_format = TJPF_GRAY;
        else if (shape->bytes_per_channel == 1 && shape->channels == 4)
            pixel_format = TJPF_RGBA;
        else
            return false;

        int success = tjDecompress2( //
                          get_jpeg_ctx(ctx_ptr),
                          (uint8_t*)data,
                          (unsigned long)len,
                          (uint8_t*)output,
                          shape->width,
                          0,
                          shape->height,
                          pixel_format,
                          0) == 0;
        return success;
    }

    case tenpack_png_k: {
        struct spng_ihdr image_header;
        spng_ctx* spng_context = get_png_ctx(ctx_ptr);
        spng_set_png_buffer(spng_context, data, len);
        spng_get_ihdr(spng_context, &image_header);
        enum spng_format fmt;

        if (shape->bytes_per_channel == 1 && shape->channels == 4)
            fmt = SPNG_FMT_RGBA8;
        else if (shape->bytes_per_channel == 1 && shape->channels == 3)
            fmt = SPNG_FMT_RGB8;
        else if (shape->bytes_per_channel == 2 && shape->channels == 2)
            fmt = SPNG_FMT_GA16;
        else if (shape->bytes_per_channel == 1 && shape->channels == 2)
            fmt = SPNG_FMT_GA8;
        else if (shape->bytes_per_channel == 1 && shape->channels == 1)
            fmt = SPNG_FMT_G8;
        else if (shape->bytes_per_channel == 2 && shape->channels == 4)
            fmt = SPNG_FMT_RGBA16;
        else
            return false;

        int success = spng_decode_image(spng_context, output, size_bytes(shape, format), fmt, 0) == 0;
        return success;
    }

    case tenpack_gif_k: {
        int* delays = NULL;
        int comp = 0, req_comp = 3;
        int width = 0, height = 0, frames = 0;
        uint8_t* buffer = stbi_load_gif_from_memory( //
            (uint8_t*)data,
            len,
            &delays,
            &width,
            &height,
            &frames,
            &comp,
            req_comp);

        if (!buffer)
            return false;
        memcpy(output, buffer, size_bytes(shape, format));
        return 1;
    }

    case tenpack_wav_k: {
        int success = drwav_init_memory(get_wav_ctx(ctx_ptr), data, len, NULL);
        if (!success)
            return false;
        return drwav_read_pcm_frames(get_wav_ctx(ctx_ptr), shape->frames, output) == shape->frames;
    }

    default: return false;
    }
}

void tenpack_context_free(tenpack_ctx_t context) {
    if (!context)
        return;
    ctx_t* ctx_ptr = (ctx_t*)(context);
    if (ctx_ptr->jpeg_)
        tjDestroy(ctx_ptr->jpeg_);
    if (ctx_ptr->png_)
        spng_ctx_free(ctx_ptr->png_);
    if (ctx_ptr->wav_state)
        drwav_uninit(&(ctx_ptr->wav_));
    free(ctx_ptr);
}
