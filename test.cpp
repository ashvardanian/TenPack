#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "tenpack.h"

int main() {

    std::vector<std::string> paths {"/home/arman/vsCode/Ten-Pack/TenPack/assets/jpg_file.jpg",
                                    "/home/arman/vsCode/Ten-Pack/TenPack/assets/png_file.png",
                                    "/home/arman/vsCode/Ten-Pack/TenPack/assets/gif_file.gif",
                                    "/home/arman/vsCode/Ten-Pack/TenPack/assets/bmp_file.bmp",
                                    "/home/arman/vsCode/Ten-Pack/TenPack/assets/avi_file.avi",
                                    "/home/arman/vsCode/Ten-Pack/TenPack/assets/mpeg_file.mp4"};

    tenpack_format_t frmt;
    std::fstream file;
    std::string buffer;
    std::string line;
    size_t* dimensios = new size_t(3);
    // size_t position = 0;

    for (int idx = 0; idx < paths.size(); ++idx) {
        file.open(paths[idx], std::fstream::in | std::fstream::binary);

        if (file.is_open()) {
            while (!file.eof()) {
                getline(file, line);
                buffer.append(line.begin(), line.end());
            }
            auto file_content = (unsigned char*)buffer.data();
            auto size = buffer.size();
            void* buff;
            tenpack_guess_format(file_content, size, &frmt);
            tenpack_guess_dimensions(file_content, size, frmt, dimensios);
            // tenpack_unpack(file_content,size,frmt,nullptr,buff,dimensios[0]);
            std::cout << frmt << std::endl;
            file.close();
            buffer.clear();
        }
    }
}