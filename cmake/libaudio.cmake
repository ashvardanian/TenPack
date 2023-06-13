include(ExternalProject)
find_package(Git REQUIRED)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libaudio-src")
set(SOURCE_DIR "${PREFIX}/libaudio-src")

ExternalProject_Add(
    ep-libaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG 0.11.17

    PREFIX ${PREFIX}
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    SOURCE_DIR ${SOURCE_DIR}
    STAMP_DIR ${PREFIX}/libaudio-stamp
    TMP_DIR ${PREFIX}/libaudio-tmp

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(LIBAUDIO_INCLUDE_DIR ${SOURCE_DIR})
include_directories(${LIBAUDIO_INCLUDE_DIR})