cmake_minimum_required (VERSION 2.8)

project (vpp)

include_directories(/usr/include/eigen3)

add_definitions(-std=c++1y -g)
add_subdirectory(tests)

install(DIRECTORY vpp DESTINATION include
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY vpp DESTINATION include
  FILES_MATCHING PATTERN "*.hpp")
