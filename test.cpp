#include <fstream>
#include <vector>
#include <string>

#include "tenpack.h"

int main() {

    std::vector<std::string> paths {
        "assets/jpg_file.jpg",
        "assets/png_file.png",
        "assets/gif_file.gif",
        "assets/bmp_file.bmp",
        "assets/avi_file.avi",
        "assets/mpeg_file.mp4",
    };

    for (auto const& path : paths) {

        // Read entire file
        std::ifstream file(path, std::ifstream::binary);
        std::vector<char> input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // We are going to need temporary memory
        tenpack_ctx_t ctx;

        // 1
        tenpack_format_t format;
        if (!tenpack_guess_format(input.data(), input.size(), &format, &ctx))
            throw std::invalid_argument("Can't guess file format:" + path);

        // 2
        tenpack_dimensions_t dims;
        if (!tenpack_guess_dimensions(input.data(), input.size(), format, &dims, &ctx))
            throw std::invalid_argument("Can't guess dimensions:" + path);

        // 3
        std::vector<char> unpacked(dims.width * dims.height * dims.frames * dims.channels * dims.bytes_per_channel);
        if (!tenpack_unpack(input.data(), input.size(), format, &dims, unpacked.data(), &ctx))
            throw std::invalid_argument("Can't export:" + path);

        tenpack_context_free(ctx);
    }
}