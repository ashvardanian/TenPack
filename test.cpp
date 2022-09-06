#include <fstream>
#include <cstring>
#include <vector>

#include "tenpack.h"

bool decode(std::string const& path, std::vector<uint8_t>& unpacked, tenpack_ctx_t ctx, tenpack_dimensions_t& dims) {

    // Read entire file
    std::ifstream file(path, std::ifstream::binary);
    std::vector<uint8_t> input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 1
    tenpack_format_t format;
    if (!tenpack_guess_format(input.data(), input.size(), &format, &ctx))
        throw std::invalid_argument("Can't guess file format:" + path);

    // 2
    if (!tenpack_guess_dimensions(input.data(), input.size(), format, &dims, &ctx))
        throw std::invalid_argument("Can't guess dimensions:" + path);

    // 3
    unpacked.resize(dims.width * dims.height * dims.frames * dims.channels * dims.bytes_per_channel);
    if (!tenpack_unpack(input.data(), input.size(), format, &dims, unpacked.data(), &ctx))
        throw std::invalid_argument("Can't export:" + path);

    return true;
}

bool check_output_equality(std::vector<std::string> const& paths) {

    std::vector<std::vector<uint8_t>> unpacked_buffers(paths.size());
    std::vector<uint8_t> unpacked;

    size_t idx = 0;
    bool success = false;
    bool success_decode = false;

    // We are going to need temporary memory
    tenpack_ctx_t ctx = nullptr;

    tenpack_dimensions_t dims;

    for (auto const& path : paths)
        success_decode = decode(path, unpacked, ctx, dims);

    for (size_t idx = 0; idx < unpacked_buffers.size() - 1; ++idx)
        success = std::memcmp( //
                      unpacked_buffers[idx].data(),
                      unpacked_buffers[idx + 1].data(),
                      unpacked_buffers[idx].size()) == false;

    tenpack_context_free(ctx);
    return success;
}

int main(int argc, char* argv[]) {

    std::vector<std::string> paths_white {//
                                          "assets/jpeg_white.jpg",
                                          "assets/png_white.png"};

    std::vector<std::string> paths_black {//
                                          "assets/jpeg_black.jpg",
                                          "assets/png_black.png"};

    bool success_white = check_output_equality(paths_white);
    bool success_black = check_output_equality(paths_black);

    return 0;
}