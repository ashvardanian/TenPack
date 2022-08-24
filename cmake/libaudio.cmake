include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    ep-libaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git

    PREFIX "_deps"
    DOWNLOAD_DIR "_deps/libaudio-src"
    SOURCE_DIR "_deps/libaudio-src"
    STAMP_DIR "_deps/libaudio-stamp"
    TMP_DIR "_deps/libaudio-tmp"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(LIBAUDIO_INCLUDE_DIR _deps/libaudio-src)
include_directories(${LIBAUDIO_INCLUDE_DIR})