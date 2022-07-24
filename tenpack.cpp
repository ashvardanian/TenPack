
constexpr char prefix_jpeg_k[3] {0xFF, 0xD8, 0xFF};
constexpr char prefix_jpeg2000_k[13] {0x0, 0x0, 0x0, 0xC, 0x6A, 0x50, 0x20, 0x20, 0xD, 0xA, 0x87, 0xA, 0x0};
constexpr char prefix_png_k[4] {0x89, 0x50, 0x4E, 0x47};
constexpr char prefix_gif_k[3] {0x47, 0x49, 0x46};
constexpr char prefix_bmp_k[2] {0x42, 0x4D};
constexpr char prefix_jxr_k[3] {0x49, 0x49, 0xBC};
constexpr char prefix_psd_k[4] {0x38, 0x42, 0x50, 0x53};
constexpr char prefix_ico_k[4] {0x00, 0x00, 0x01, 0x00};
constexpr char prefix_dwg_k[4] {0x41, 0x43, 0x31, 0x30};

// Relevant OpenCV funcs:
// https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga5a0acefe5cbe0a81e904e452ec7ca733
// https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga26a67788faa58ade337f8d28ba0eb19e
// https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56
// https://docs.opencv.org/4.x/d0/d61/group__cudacodec.html