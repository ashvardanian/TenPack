include(ExternalProject)
find_package(Git REQUIRED)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libstb-src")
set(SOURCE_DIR "${PREFIX}/libstb-src")

ExternalProject_Add(
    ep-libstb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master

    PREFIX ${PREFIX}
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    SOURCE_DIR ${SOURCE_DIR}
    STAMP_DIR ${PREFIX}/libstb-stamp
    TMP_DIR ${PREFIX}/libstb-tmp

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(LIBstb_INCLUDE_DIR ${SOURCE_DIR})
include_directories(${LIBstb_INCLUDE_DIR})