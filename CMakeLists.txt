cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(BIN_DIR ${rope_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -O0 -g")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_MODULE_PATH "~/sync/root/alaroldai/conf/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

include_directories(src)

set(srcs
    src/slice.hpp
    src/rope_node.hpp
    src/rope.hpp
    src/fibonacci.hpp src/fibonacci.cc
    src/rope_iter.hpp
    src/rope_global_conf.hpp
    src/utf8.hpp
    src/utf8.cc
    src/measure.hpp)

add_library(rope SHARED
    ${srcs}
)

add_executable(rope_demo src/main.cc)
target_link_libraries(rope_demo rope)
