cmake_minimum_required(VERSION 3.1)
project(TenPack)

option(TENPACK_BUILD_PYTHON "Building Python bidings for all SDKs")
option(TENPACK_BUILD_TESTS "Building C/C++/python native tests" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_CACHEFILE_DIR "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib" CACHE PATH "Path to static libs")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib" CACHE PATH "Path to static libs")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin")

message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(FetchContent)
include_directories(include/)

set(FETCHCONTENT_QUIET OFF)

include("cmake/libturbojpeg.cmake")
include("cmake/libdr_libs.cmake")
include("cmake/pybind11.cmake")
include("cmake/libspng.cmake")
include("cmake/libstb.cmake")
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -g")

find_package(Python 3.10.6 COMPONENTS Interpreter Development NumPy REQUIRED)
include_directories(${Python_NumPy_INCLUDE_DIRS})
include_directories(${pybind11_INCLUDE_DIRS})

add_library(tenpack STATIC src/tenpack.cpp)

target_link_libraries(tenpack libturbojpeg libspng_static ZLIB::ZLIB dl m pthread)
get_target_property(TARGET_LIBRARIES tenpack LINK_LIBRARIES)
add_dependencies(tenpack ep-libspng ep-libturbojpeg ep-libstb  ep-libdr_libs)

if(${TENPACK_BUILD_TESTS})
    add_executable(tenpack_test tests/test.cpp)
    target_link_libraries(tenpack_test tenpack)
    add_dependencies(tenpack_test tenpack)
endif()

if(${TENPACK_BUILD_PYTHON})
    file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/tenpack)
    pybind11_add_module(tenpack_module MODULE python/lib.cpp)
    target_link_libraries(tenpack_module PRIVATE tenpack ${TARGET_LIBRARIES} ${PYTHON_LIBRARIES})
    target_include_directories(tenpack_module PUBLIC python/)
endif()
