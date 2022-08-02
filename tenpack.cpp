#include <turbojpeg.h>

#include "tenpack.h"

constexpr unsigned char prefix_jpeg_k[3] {0xFF, 0xD8, 0xFF};
constexpr unsigned char prefix_jpeg2000_k[13] {0x0, 0x0, 0x0, 0xC, 0x6A, 0x50, 0x20, 0x20, 0xD, 0xA, 0x87, 0xA, 0x0};
constexpr unsigned char prefix_png_k[4] {0x89, 0x50, 0x4E, 0x47};
constexpr unsigned char prefix_gif_k[3] {0x47, 0x49, 0x46};
constexpr unsigned char prefix_bmp_k[2] {0x42, 0x4D};
constexpr unsigned char prefix_jxr_k[3] {0x49, 0x49, 0xBC};
constexpr unsigned char prefix_psd_k[4] {0x38, 0x42, 0x50, 0x53};
constexpr unsigned char prefix_ico_k[4] {0x00, 0x00, 0x01, 0x00};
constexpr unsigned char prefix_dwg_k[4] {0x41, 0x43, 0x31, 0x30};

// Relevant OpenCV funcs:
// https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga5a0acefe5cbe0a81e904e452ec7ca733
// https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga26a67788faa58ade337f8d28ba0eb19e
// https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56
// https://docs.opencv.org/4.x/d0/d61/group__cudacodec.html

void tenpack_guess_dimensions( //
    void* content_bytes,
    size_t content_length,
    tenpack_format_t format,
    size_t* guessed_dimensions) {

    // There is some documentation in libjpeg.txt. libjpeg is a very low-level, steep-learning-curve,
    // old school c library. To use it effectively you need to be familiar with setjmp and longjmp,
    // c structure layouts, function pointers, and lots of other low-level C stuff. It's a bear to
    // work with but possible to do a great deal with minimal resource usage.
}
