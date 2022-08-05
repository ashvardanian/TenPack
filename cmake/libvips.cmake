## To build, you need gobject-introspection, python-gi-dev, and libgirepository1.0-dev.

include(ExternalProject)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libvips-src")
set(SOURCE_DIR "${PREFIX}/libvips-src")


ExternalProject_Add(
    ep-libvips
    URL https://github.com/libvips/libvips/releases/download/v8.13.0/vips-8.13.0.tar.gz

    CONFIGURE_COMMAND cd ${SOURCE_DIR} && meson build
    BUILD_COMMAND cd ${SOURCE_DIR} && ninja -C build
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBVIPS_INCLUDE_DIR _deps/libvips-src/libvips/include/vips)
set(LIBVIPS_LIB_DIR _deps/libvips-src/build/libvips)

include_directories(${LIBVIPS_INCLUDE_DIR})
link_directories(${LIBVIPS_LIB_DIR})

add_library(libvips STATIC IMPORTED)
set_property(TARGET libvips PROPERTY
             IMPORTED_LOCATION "${LIBVIPS_LIB_DIR}/libvips.so")