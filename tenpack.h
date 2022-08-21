/**
 * @file tenpack.h
 * @author Ashot Vardanian
 * @brief
 * @version 0.1
 * @date 2022-08-02
 *
 * @brief Micro-library exporting variable-length encoded data into regular Tensors,
 * that Machine Learning libraries can accept. Can submit both singular media objects,
 * and batches, as well as streams, decoding and reshaping frames along the way.
 * TenPack has very little logic internally, and mostly just links codecs/libs together.
 * Supports Apache Arrow inputs.
 *
 * @section Supported Formats
 * Images: JPEG, PNG, GIF?
 * Audios: WAV?
 * Videos: MPEG4?
 *
 * @section Supported Transforms
 * Images: resize, transpose.
 * Audios: resize, fft.
 * Videos: resize, transpose.
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
    // A spatial dimension.
    size_t width;
    // A spatial dimension.
    size_t height;
    // A spatial dimension.
    size_t channels;
    // A temporal dimension.
    size_t frames;
    // The resolution of every exported numerical value.
    size_t bytes_per_scalar;
};

typedef void const* tenpack_input_t;
typedef void* tenpack_output_t;
typedef void* tenpack_ctx_t;

bool tenpack_context_free(tenpack_ctx_t);

/**
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data       Pointer to the start of binary media data.
 * @param[in] len        Length of the binary blob.
 * @param[inout] format  Pointer, where the guess will be written.
 * @param[inout] context Pointer, where the guess will be written.
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
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data       Pointer to the start of binary media data.
 * @param[in] len        Length of the binary blob.
 * @param[in] format     The format of data in `[data, data+len)`.
 * @param[inout]dims     Output dimensions of image.
 *                       > For JPEG and PNG, 3 dims: width, height, channels.
 *                       > For GIF, 3 dims: width, height, frames.
 *                       > For AVI, 4 dims: width, height, channels, frames.
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
 * @brief Guesses the format of binary data just by comparing various binary signatures.
 *
 * @param[in] data       Pointer to the start of binary media data.
 * @param[in] len        Length of the binary blob.
 * @param[in] format     The format of data in `[data, data+len)`.
 *
 * @return true          If the type was successfully guessed.
 * @return false         If error occurred.
 */
bool tenpack_unpack( //
    tenpack_input_t const data,
    size_t const len,
    tenpack_format_t const format,
    tenpack_dimensions_t const* output_dimensions,
    tenpack_output_t output,
    tenpack_ctx_t* context);

/**
 * @brief Changes/transposes the content order in an Array-of-Structures to
 * Structure-of-Arrays form, more familiar to machine-learning libraries.
 * Overall volume of content-populated memory will remain the same, but
 * intermediate allocations may still take place for the @param context.
 */
bool tenpack_transpose( //
    tenpack_dimensions_t const* dimensions,
    tenpack_output_t output,
    tenpack_ctx_t* context);

/**
 * @brief Accepts an Apache Arrow binary "StringsArray" and performs the
 * entire introspection and extraction pipeline on each of its members:
 * > guess format,
 * > guess dimensions,
 * > unpack,
 * > transpose.
 */
bool tenpack_export( //
    tenpack_input_t const input_tape_start,
    uint32_t const* input_tape_offsets,
    size_t const count,
    tenpack_output_t output_tensor_start,
    tenpack_dimensions_t const* output_sample_dimensions,
    tenpack_ctx_t* context);

bool tenpack_context_free(tenpack_ctx_t);

#ifdef __cplusplus
} /* end extern "C" */
#endif
