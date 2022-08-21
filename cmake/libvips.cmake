# To build, you need gobject-introspection, python-gi-dev, and libgirepository1.0-dev.

include(ExternalProject)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libvips-src")
set(SOURCE_DIR "${PREFIX}/libvips-src")
set(BINARY_DIR "${PREFIX}/libvips-build")

ExternalProject_Add(
    ep-libvips
    URL https://github.com/libvips/libvips/releases/download/v8.13.0/vips-8.13.0.tar.gz

    PREFIX ${PREFIX}
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    SOURCE_DIR ${SOURCE_DIR}
    BINARY_DIR ${BINARY_DIR}

    CONFIGURE_COMMAND cd ${BINARY_DIR} && meson build ${SOURCE_DIR}
    BUILD_COMMAND cd ${BINARY_DIR} && ninja -C build
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBVIPS_INCLUDE_DIRS  "${SOURCE_DIR}/cplusplus/include/;${SOURCE_DIR}/libvips/include/;${BINARY_DIR}/build/libvips/include/")

set(LIBVIPS_LIB_DIR ${BINARY_DIR}/build/cplusplus/)

include_directories(${LIBVIPS_INCLUDE_DIRS})
link_directories(${LIBVIPS_LIB_DIR})

add_library(libvips-cpp SHARED IMPORTED)
set_property(TARGET libvips-cpp PROPERTY IMPORTED_LOCATION "${LIBVIPS_LIB_DIR}/libvips-cpp.so")