include(ExternalProject)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libspng-src")
set(SOURCE_DIR "${PREFIX}/libspng-src")
set(BINARY_DIR "${PREFIX}/libspng-build")

ExternalProject_Add(
    ep-libspng
    URL https://github.com/randy408/libspng/archive/refs/tags/v0.7.2.tar.gz

    PREFIX "${PREFIX}"
    DOWNLOAD_DIR "${DOWNLOAD_DIR}"
    LOG_DIR "${PREFIX}/libspng-log"
    STAMP_DIR "${PREFIX}/libspng-stamp"
    TMP_DIR "${PREFIX}/libspng-tmp"
    SOURCE_DIR "${SOURCE_DIR}"
    INSTALL_DIR "${PREFIX}/libspng-install"
    BINARY_DIR "${BINARY_DIR}"

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBSPNG_INCLUDE_DIR ${BINARY_DIR}/clean/include)
set(LIBSPNG_LIB_DIR ${BINARY_DIR}/clean/lib)

include_directories(${LIBSPNG_INCLUDE_DIR})
link_directories(${LIBSPNG_LIB_DIR})

add_library(libspng_static STATIC IMPORTED)
set_property(TARGET libspng_static PROPERTY
             IMPORTED_LOCATION "${LIBSPNG_LIB_DIR}/libspng_static.a")