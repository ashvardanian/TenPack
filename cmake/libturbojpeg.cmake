include(ExternalProject)

ExternalProject_Add(
    ep-libturbojpeg
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.3.tar.gz

    PREFIX "_deps"
    DOWNLOAD_DIR "_deps/libturbojpeg-src"
    LOG_DIR "_deps/libturbojpeg-log"
    STAMP_DIR "_deps/libturbojpeg-stamp"
    TMP_DIR "_deps/libturbojpeg-tmp"
    SOURCE_DIR "_deps/libturbojpeg-src"
    INSTALL_DIR "_deps/libturbojpeg-install"
    BINARY_DIR "_deps/libturbojpeg-build"

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBTURBOJPEG_INCLUDE_DIR _deps/libturbojpeg-build/clean/include)
set(LIBTURBOJPEG_LIB_DIR _deps/libturbojpeg-build/clean/lib)

include_directories(${LIBTURBOJPEG_INCLUDE_DIR})
link_directories(${LIBTURBOJPEG_LIB_DIR})