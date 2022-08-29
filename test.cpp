#include <fstream>
#include <cstring>
#include <vector>

#include "tenpack.h"

bool decode(std::vector<std::string> const& paths, std::vector<std::vector<u_int8_t>>& unpacked_buffers) {
    unpacked_buffers.resize(paths.size());
    size_t output_size = 0;
    size_t idx = 0;

    for (auto const& path : paths) {

        // Read entire file
        std::ifstream file(path, std::ifstream::binary);
        std::vector<u_int8_t> input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // We are going to need temporary memory
        tenpack_ctx_t ctx = nullptr;

        // 1
        tenpack_format_t format;
        if (!tenpack_guess_format(input.data(), input.size(), &format, &ctx))
            throw std::invalid_argument("Can't guess file format:" + path);

        // 2
        tenpack_dimensions_t dims;
        if (!tenpack_guess_dimensions(input.data(), input.size(), format, &dims, &ctx))
            throw std::invalid_argument("Can't guess dimensions:" + path);

        // 3
        std::vector<u_int8_t> unpacked(dims.width * dims.height * dims.frames * dims.channels * dims.bytes_per_channel);
        output_size = unpacked.size();
        if (!tenpack_unpack(input.data(), input.size(), format, &dims, unpacked.data(), &ctx))
            throw std::invalid_argument("Can't export:" + path);

        unpacked_buffers[idx].resize(output_size);
        std::memcpy(unpacked_buffers[idx++].data(), unpacked.data(), output_size);
        tenpack_context_free(ctx);
    }

    return true;
}

bool check_output_equality(std::vector<std::vector<u_int8_t>>& unpacked_buffers) {

    bool success = false;

    for (size_t idx = 0; idx < unpacked_buffers.size() - 1; ++idx)
        success = std::memcmp( //
                      unpacked_buffers[idx].data(),
                      unpacked_buffers[idx + 1].data(),
                      unpacked_buffers[idx].size()) == false;

    return success;
}

int main(int argc, char* argv[]) {

    std::vector<std::string> paths_white {//
                                          "assets/jpeg_white.jpg",
                                          "assets/png_white.png"};

    std::vector<std::string> paths_black {//
                                          "assets/jpeg_black.jpg",
                                          "assets/png_black.png"};

    std::vector<std::vector<u_int8_t>> unpacked_buffers;
    
    decode(paths_white, unpacked_buffers);
    bool success_white = check_output_equality(unpacked_buffers);

    decode(paths_black, unpacked_buffers);
    bool success_black = check_output_equality(unpacked_buffers);

    return 0;
}