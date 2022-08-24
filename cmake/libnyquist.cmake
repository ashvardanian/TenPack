include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    ep-libnyquist
    GIT_REPOSITORY https://github.com/ddiakopoulos/libnyquist.git

    PREFIX "_deps"
    DOWNLOAD_DIR "_deps/libnyquist-src"
    LOG_DIR "_deps/libnyquist-log"
    STAMP_DIR "_deps/libnyquist-stamp"
    TMP_DIR "_deps/libnyquist-tmp"
    SOURCE_DIR "_deps/libnyquist-src"
    INSTALL_DIR "_deps/libnyquist-install"
    BINARY_DIR "_deps/libnyquist-build"

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBNYQUIST_INCLUDE_DIR _deps/libnyquist-src/include/libnyquist)
set(LIBNYQUIST_LIB_DIR _deps/libnyquist-build/clean/lib)

include_directories(${LIBNYQUIST_INCLUDE_DIR})
link_directories(${LIBNYQUIST_LIB_DIR})

add_library(libnyquist STATIC IMPORTED)
set_property(TARGET libnyquist PROPERTY
             IMPORTED_LOCATION "${LIBNYQUIST_LIB_DIR}/liblibnyquist.a")