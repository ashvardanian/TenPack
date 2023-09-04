include(ExternalProject)

set(PREFIX "${CMAKE_BINARY_DIR}/_deps")
set(DOWNLOAD_DIR "${PREFIX}/ffmpeg-src")
set(SOURCE_DIR "${PREFIX}/ffmpeg-src")
set(BINARY_DIR "${PREFIX}/ffmpeg-build")

ExternalProject_Add(
  ep-ffmpeg
  URL https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n3.4.13.tar.gz

  PREFIX ${PREFIX}
  DOWNLOAD_DIR ${DOWNLOAD_DIR}
  SOURCE_DIR ${SOURCE_DIR}
  BINARY_DIR ${BINARY_DIR}

  CONFIGURE_COMMAND cd ${SOURCE_DIR} && ./configure --prefix=../ffmpeg-build && make
  BUILD_COMMAND cd ${SOURCE_DIR} && make install
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""

  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=clean
)

set(FFMPEG_INCLUDE_DIR ${BINARY_DIR}/include)
set(FFMPEG_LIB_DIR ${BINARY_DIR}/lib)

add_library(libavcodec STATIC IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavcodec.a)

add_library(libavformat STATIC IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavformat.a)

add_library(libavutil STATIC IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavutil.a)

add_library(libavdevice STATIC IMPORTED)
set_target_properties(libavdevice PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavdevice.a)

include_directories(${FFMPEG_INCLUDE_DIR})