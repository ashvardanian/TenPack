include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    ep-libaudio
    GIT_REPOSITORY https://github.com/ddiakopoulos/libnyquist.git
    
    PREFIX "_deps"
    DOWNLOAD_DIR "_deps/libaudio-src"
    LOG_DIR "_deps/libaudio-log"
    STAMP_DIR "_deps/libaudio-stamp"
    TMP_DIR "_deps/libaudio-tmp"
    SOURCE_DIR "_deps/libaudio-src"
    INSTALL_DIR "_deps/libaudio-install"
    BINARY_DIR "_deps/libaudio-build"

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBAUDIO_INCLUDE_DIR _deps/libaudio-build/clean/include)
set(LIBAUDIO_LIB_DIR _deps/libaudio-build/clean/lib)

include_directories(${LIBAUDIO_INCLUDE_DIR})
link_directories(${LIBAUDIO_LIB_DIR})

add_library(libaudio STATIC IMPORTED)
set_property(TARGET libaudio PROPERTY
             IMPORTED_LOCATION "${LIBAUDIO_LIB_DIR}/liblibnyquist.a")