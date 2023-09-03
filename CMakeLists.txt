cmake_minimum_required(VERSION 3.11)
project(TenPack)

option(TENPACK_BUILD_PYTHON "Building Python bidings for all SDKs")
option(TENPACK_BUILD_TESTS "Building C/C++/python native tests" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_CACHEFILE_DIR "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib" CACHE PATH "Path to static libs")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib" CACHE PATH "Path to static libs")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin")

message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message("CMAKE_C_COMPILER_VERSION: ${CMAKE_C_COMPILER_VERSION}")
message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
include_directories(include/)

set(FETCHCONTENT_QUIET OFF)

include("cmake/libturbojpeg.cmake")
include("cmake/libdr_libs.cmake")
include("cmake/libspng.cmake")
include("cmake/libstb.cmake")
find_package(ZLIB REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3 -g")

find_package(Python 3.10.6 COMPONENTS Interpreter Development NumPy REQUIRED)
include_directories(${Python_NumPy_INCLUDE_DIRS})

add_library(tenpack STATIC src/tenpack.c)

target_link_libraries(tenpack libturbojpeg libspng_static ZLIB::ZLIB dl m pthread)
get_target_property(TARGET_LIBRARIES tenpack LINK_LIBRARIES)
add_dependencies(tenpack ep-libspng ep-libturbojpeg ep-libstb  ep-libdr_libs)

if(${TENPACK_BUILD_TESTS})
    add_executable(tenpack_test tests/test.cpp)
    target_link_libraries(tenpack_test tenpack)
    add_dependencies(tenpack_test tenpack)
endif()

if(${TENPACK_BUILD_PYTHON})
    find_package(PythonLibs REQUIRED)
    add_library(tenpack_python MODULE python/lib.c)

    target_link_libraries(tenpack_python PRIVATE tenpack ${TARGET_LIBRARIES} ${PYTHON_LIBRARIES})
    target_include_directories(tenpack_python PUBLIC python/ ${PYTHON_INCLUDE_DIRS})
    set_target_properties(tenpack_python PROPERTIES PREFIX "")
    if(WIN32)
        set_target_properties(tenpack_python PROPERTIES SUFFIX ".pyd")
    else()
        set_target_properties(tenpack_python PROPERTIES SUFFIX ".so")
    endif()
endif()

