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

/**
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data      Pointer to the start of binary media data.
 * @param[in] len       Length of the binary blob.
 * @param[inout] format Pointer, where the guess will be written.
 * @return true         If the type was successfully guessed.
 * @return false        If error occurred.
 */
bool tenpack_guess_format( //
    void* data,
    size_t len,
    tenpack_format_t* format);

/**
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data      Pointer to the start of binary media data.
 * @param[in] len       Length of the binary blob.
 * @param[in] format    The format of data in `[data, data+len)`.
 * @param[inout]dims    Output dimensions of image.
 *                      > For JPEG and PNG, 3 dims: width, height, channels.
 *                      > For GIF, 3 dims: width, height, frames.
 *                      > For AVI, 4 dims: width, height, channels, frames.
 * @return true         If the type was successfully guessed.
 * @return false        If error occurred.
 */
bool tenpack_guess_dimensions( //
    void* data,
    size_t len,
    tenpack_format_t format,
    size_t* dims);

/**
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data      Pointer to the start of binary media data.
 * @param[in] len       Length of the binary blob.
 * @param[in] format    The format of data in `[data, data+len)`.
 *
 * @return true         If the type was successfully guessed.
 * @return false        If error occurred.
 */
bool tenpack_unpack( //
    void*,
    size_t,
    tenpack_format_t,
    size_t*,
    void*,
    size_t);

#ifdef __cplusplus
} /* end extern "C" */
#endif
