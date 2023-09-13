include(ExternalProject)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libturbojpeg-src")
set(SOURCE_DIR "${PREFIX}/libturbojpeg-src")
set(BINARY_DIR "${PREFIX}/libturbojpeg-build")

ExternalProject_Add(
    ep-libturbojpeg
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.3.tar.gz

    PREFIX ${PREFIX}
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    SOURCE_DIR ${SOURCE_DIR}
    BINARY_DIR ${BINARY_DIR}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean -DCMAKE_POSITION_INDEPENDENT_CODE=ON
)

set(LIBTURBOJPEG_INCLUDE_DIR ${BINARY_DIR}/clean/include)
set(LIBTURBOJPEG_LIB_DIR ${BINARY_DIR}/clean/lib)

include_directories(${LIBTURBOJPEG_INCLUDE_DIR})
link_directories(${LIBTURBOJPEG_LIB_DIR})

add_library(libturbojpeg STATIC IMPORTED)
set_property(TARGET libturbojpeg PROPERTY
             IMPORTED_LOCATION "${LIBTURBOJPEG_LIB_DIR}/libturbojpeg.a")