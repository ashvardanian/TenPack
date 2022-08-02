/**
 * @file tenpack.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-08-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum tenpack_format_t {
    tenpack_format_unknown_k,

    // Images:
    tenpack_jpeg_k,
    tenpack_jpeg2000_k,
    tenpack_png_k,
    tenpack_gif_k,
    tenpack_bmp_k,
    tenpack_ico_k,

    // Audio
    tenpack_wav_k,

    // Video
    tenpack_mpeg4_k,
    tenpack_avi_k,
};

void tenpack_guess_format(void*, size_t, tenpack_format_t*);

void tenpack_guess_dimensions(void*, size_t, tenpack_format_t, size_t*);

void tenpack_upack(void*, size_t, tenpack_format_t, size_t* slice, void* output_begin, size_t output_stride);

#ifdef __cplusplus
} /* end extern "C" */
#endif
