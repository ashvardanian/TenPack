#include <fstream>
#include <cstring>
#include <vector>

#include "tenpack.h"

enum colors {
    black = 0x00,
    white = 0xFF,
};

void decode(std::string const& path, std::vector<uint8_t>& unpacked, tenpack_ctx_t ctx, tenpack_dimensions_t& dims) {

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
}

bool check_output_equality(std::vector<uint8_t> const& buffer, colors color, std::size_t channels_count) {
    bool success = true;

    if (channels_count == 4) {
        for (size_t idx = 0; idx < buffer.size(); idx += 4) {
            if (!success)
                return false;
            for (size_t rgb = 0; rgb < 3; ++rgb)
                success = buffer[idx + rgb] == color;
        }
        return success;
    }
    for (size_t idx = 0; idx < buffer.size(); ++idx) {
        if (!success)
            return false;
        success = buffer[idx] == color;
    }
    return success;
}

int main(int argc, char* argv[]) {
    std::vector<uint8_t> unpacked;

    // We are going to need temporary memory
    tenpack_ctx_t ctx = nullptr;

    tenpack_dimensions_t dims;

    std::vector<std::string> paths_white {//
                                          "assets/jpeg_white.jpg",
                                          "assets/png_white.png"};

    std::vector<std::string> paths_black {//
                                          "assets/jpeg_black.jpg",
                                          "assets/png_black.png"};

    bool success_white = true;
    bool success_black = true;

    // White images
    for (size_t idx = 0; idx < paths_white.size() && success_white; ++idx) {
        decode(paths_white[idx], unpacked, ctx, dims);
        success_white = check_output_equality(unpacked, colors::white, dims.channels);
        unpacked.clear();
    }
    ///

    // Black images
    for (size_t idx = 0; idx < paths_black.size() && success_black; ++idx) {
        decode(paths_black[idx], unpacked, ctx, dims);
        success_black = check_output_equality(unpacked, colors::black, dims.channels);
        unpacked.clear();
    }
    ///

    tenpack_context_free(ctx);
    return 0;
}