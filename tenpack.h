
#pragma once

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

void tenpack_guess_dimensions(void*, size_t, tenpack_format_t, size_t);

void tenpack_upack(void*, size_t, tenpack_format_t, size_t* slice, void* output_begin, size_t output_stride);
