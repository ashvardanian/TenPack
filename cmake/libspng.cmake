include(ExternalProject)

ExternalProject_Add(
    ep-libspng
    URL https://github.com/randy408/libspng/archive/refs/tags/v0.7.2.tar.gz

    PREFIX "_deps"
    DOWNLOAD_DIR "_deps/libspng-src"
    LOG_DIR "_deps/libspng-log"
    STAMP_DIR "_deps/libspng-stamp"
    TMP_DIR "_deps/libspng-tmp"
    SOURCE_DIR "_deps/libspng-src"
    INSTALL_DIR "_deps/libspng-install"
    BINARY_DIR "_deps/libspng-build"

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBSPNG_INCLUDE_DIR _deps/libspng-build/clean/include)
set(LIBSPNG_LIB_DIR _deps/libspng-build/clean/lib)

include_directories(${LIBSPNG_INCLUDE_DIR})
link_directories(${LIBSPNG_LIB_DIR})

add_library(libspng_static STATIC IMPORTED)
set_property(TARGET libspng_static PROPERTY IMPORTED_LOCATION "${LIBSPNG_LIB_DIR}/libspng_static.a")