cmake_minimum_required(VERSION 3.4)

project(ryu)

add_library(ryu 
    ryu/f2s.c
    ryu/d2s.c
    ryu/d2fixed.c
    ryu/generic_128.c)
target_include_directories(ryu PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_subdirectory(ryu/benchmark)