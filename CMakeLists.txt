cmake_minimum_required (VERSION 2.8)

project (vpp)

include_directories(/usr/include/eigen3)

add_definitions(-std=c++14)
add_subdirectory(tests)
add_subdirectory(examples)

install(DIRECTORY vpp DESTINATION include
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY vpp DESTINATION include
  FILES_MATCHING PATTERN "*.hpp")
