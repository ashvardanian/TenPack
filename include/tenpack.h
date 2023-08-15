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

struct tenpack_dimensions_t {
    size_t frames;
    size_t width;
    size_t height;
    size_t channels;
    size_t bytes_per_channel;
    bool is_signed = false;
};

typedef void const* tenpack_input_t;
typedef void* tenpack_ctx_t;

bool tenpack_context_free(tenpack_ctx_t);

/**
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data       Pointer to the start of binary media data.
 * @param[in] len        Length of the binary blob.
 * @param[inout] format  Object, where the guessed fromat will be written.
 * @param[inout] context A pointer to where the file handler is stored.
 *
 * @return true          If the type was successfully guessed.
 * @return false         If error occurred.
 */
bool tenpack_guess_format( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t* format,
    tenpack_ctx_t* context);

/**
 * @brief Guesses the file dimensions of binary data just by reading binary signatures.
 *
 * @param[in] data       Pointer to the start of binary media data.
 * @param[in] len        Length of the binary blob.
 * @param[in] format     The format of data in `[data, data+len)`.
 * @param[inout] dims     Output dimensions of image.
 *                       > For JPEG and PNG, 3 dims: width, height, channels.
 *                       > For GIF, 3 dims: width, height, frames.
 *                       > For AVI, 4 dims: width, height, channels, frames.
 * @param[inout] context A pointer to where the file handler is stored.
 *
 * @return true          If the type was successfully guessed.
 * @return false         If error occurred.
 */
bool tenpack_guess_dimensions( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t const format,
    tenpack_dimensions_t* dims,
    tenpack_ctx_t* context);

/**
 * @brief unpacking binary data.
 *
 * @param[in] data       Pointer to the start of binary media data.
 * @param[in] len        Length of the binary blob.
 * @param[in] format     The format of data in `[data, data+len)`.
 * @param[in] output_dimensions     Output tensor property.
 * @param[in] output     Output tensor.
 * @param[inout] context A pointer to where the file handler is stored.
 *
 * @return true          If the type was successfully guessed.
 * @return false         If error occurred.
 */
bool tenpack_unpack( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t const format,
    tenpack_dimensions_t const* output_dimensions,
    void* output,
    tenpack_ctx_t* context);

#ifdef __cplusplus
} /* end extern "C" */
#endif
