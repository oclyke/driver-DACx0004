cmake_minimum_required(VERSION 3.14)
project(dacx0004 LANGUAGES C CXX)

add_library(dacx0004_lib dacx0004.c)
target_include_directories(dacx0004_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_subdirectory(tests)
