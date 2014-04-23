cmake_minimum_required (VERSION 2.8)

project (vpp)

include_directories(/usr/include/eigen3)

add_definitions(-std=c++11 -g)
add_subdirectory(tests)
