cmake_minimum_required(VERSION 3.19)
project(UKV)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(CMAKE_CXX_STANDARD 20)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fPIC")

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

include("cmake/libturbojpeg.cmake")
include("cmake/libaudio.cmake")
include("cmake/libspng.cmake")
include("cmake/libffmpeg.cmake")
include("cmake/libnsgif.cmake")
include("cmake/pybind11.cmake")
find_package(ZLIB REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0 gio-2.0 REQUIRED)

find_package(Python 3.9 COMPONENTS Interpreter Development NumPy REQUIRED)
include_directories(${Python_NumPy_INCLUDE_DIRS})
include_directories(${pybind11_INCLUDE_DIRS})

include_directories(${GLIB_INCLUDE_DIRS})
add_library(tenpack STATIC tenpack.cpp)

target_link_libraries(tenpack libavcodec libavformat libavutil libavdevice libnsgif libturbojpeg libspng_static ZLIB::ZLIB dl m pthread ${GLIB_LIBRARIES})
target_include_directories(tenpack PRIVATE ${GLIB_INCLUDE_DIRS})

get_target_property(TARGET_LIBRARIES tenpack LINK_LIBRARIES)

add_executable(tenpack_test test.cpp)
target_link_libraries(tenpack_test tenpack)

pybind11_add_module(tenpack_module MODULE python/pybind.cpp)
target_link_libraries(tenpack_module PRIVATE tenpack ${TARGET_LIBRARIES} ${PYTHON_LIBRARIES})
target_include_directories(tenpack_module PUBLIC python/)

add_test(NAME tenpack_module_test COMMAND ${Python_EXECUTABLE} python/tests/pytest.py)

add_dependencies(tenpack ep-libspng ep-libturbojpeg ep-libnsgif)
add_dependencies(tenpack_test tenpack)