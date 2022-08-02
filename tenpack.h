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
    tenpack_bmp_k,
    tenpack_gif_k,
    tenpack_jxr_k,
    tenpack_png_k,
    tenpack_psd_k,
    tenpack_dwg_k,
    tenpack_ico_k,
    tenpack_jpeg_k,
    tenpack_jpeg2000_k,

    // Audio
    tenpack_wav_k,

    // Video
    tenpack_avi_k,
    tenpack_mpeg4_k,
};

bool tenpack_guess_format(void*, size_t, tenpack_format_t*);

bool tenpack_guess_dimensions( //
    void*,
    size_t,
    tenpack_format_t,
    size_t*);

bool tenpack_unpack( //
    void*,
    size_t,
    tenpack_format_t,
    size_t* slice,
    void* output_begin,
    size_t output_stride);

#ifdef __cplusplus
} /* end extern "C" */
#endif
