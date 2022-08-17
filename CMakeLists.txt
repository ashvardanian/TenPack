cmake_minimum_required(VERSION 3.19)
project(UKV)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CACHEFILE_DIR "${CMAKE_SOURCE_DIR}/tmp/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

include("cmake/libturbojpeg.cmake")
include("cmake/libspng.cmake")
include("cmake/libaudio.cmake")
include("cmake/libvips.cmake")
find_package(ZLIB REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib2 REQUIRED IMPORTED_TARGET glib-2.0)

include_directories(${glib2_INCLUDE_DIRS})
add_library(tenpack tenpack.cpp)
target_link_libraries(tenpack libturbojpeg libspng_static ZLIB::ZLIB libaudio  libvips libvips-cpp ${glib2_LIBRARIES})

add_executable(tenpack_test test.cpp tenpack.cpp)
target_link_libraries(tenpack_test libturbojpeg libspng_static ZLIB::ZLIB libaudio libvips libvips-cpp ${glib2_LIBRARIES})

add_dependencies(tenpack ep-libspng ep-libturbojpeg ep-libaudio  ep-libvips)
add_dependencies(tenpack_test ep-libspng ep-libturbojpeg ep-libaudio  ep-libvips)