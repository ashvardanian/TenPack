include(ExternalProject)
find_package(Git REQUIRED)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libdr_libs-src")
set(SOURCE_DIR "${PREFIX}/libdr_libs-src")

ExternalProject_Add(
    ep-libdr_libs
    GIT_REPOSITORY https://github.com/mackron/dr_libs.git
    GIT_TAG master

    PREFIX ${PREFIX}
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    SOURCE_DIR ${SOURCE_DIR}
    STAMP_DIR ${PREFIX}/libdr_libs-stamp
    TMP_DIR ${PREFIX}/libdr_libs-tmp

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(LIBdr_libs_INCLUDE_DIR ${SOURCE_DIR})
include_directories(${LIBdr_libs_INCLUDE_DIR})