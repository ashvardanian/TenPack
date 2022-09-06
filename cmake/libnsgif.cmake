include(ExternalProject)
find_package(Git REQUIRED)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/libnsgif-src")
set(SOURCE_DIR "${PREFIX}/libnsgif-src")
set(BINARY_DIR "${PREFIX}/libnsgif-build")

ExternalProject_Add(
  ep-libnsgif
  GIT_REPOSITORY https://github.com/netsurf-browser/libnsgif.git

  PREFIX ${PREFIX}
  DOWNLOAD_DIR ${DOWNLOAD_DIR}
  SOURCE_DIR ${SOURCE_DIR}
  BINARY_DIR ${BINARY_DIR}

  CONFIGURE_COMMAND cd ${SOURCE_DIR} && git clone https://github.com/pcwalton/netsurf-buildsystem.git 
  BUILD_COMMAND cd ${SOURCE_DIR} && export NSSHARED=netsurf-buildsystem && export PREFIX=../libnsgif-build && make install
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""

  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(LIBNSGIF_INCLUDE_DIR ${BINARY_DIR})
set(LIBNSGIF_LIB_DIR ${BINARY_DIR})

include_directories(${LIBNSGIF_INCLUDE_DIR})

add_library(libnsgif STATIC IMPORTED)
set_target_properties(libnsgif PROPERTIES IMPORTED_LOCATION ${LIBNSGIF_LIB_DIR}/libnsgif.a)
