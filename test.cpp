#include <iostream>
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

        // 1
        tenpack_format_t format;
        if (!tenpack_guess_format(input.data(), input.size(), &format))
            throw std::invalid_argument("Can't guess file format:" + path);

        // 2
        size_t dimensions[4] = {1, 1, 1, 1};
        if (!tenpack_guess_dimensions(input.data(), input.size(), format, dimensions))
            throw std::invalid_argument("Can't guess dimensions:" + path);

        // 3
        std::vector<char> unpacked(dimensions[0] * dimensions[1] * dimensions[2] * dimensions[3]);
        if (!tenpack_unpack(input.data(), input.size(), format, nullptr, unpacked.data(), dimensions[0]))
            throw std::invalid_argument("Can't export:" + path);
    }
}