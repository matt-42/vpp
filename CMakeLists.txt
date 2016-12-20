cmake_minimum_required (VERSION 2.8)

project (vpp)

find_package(PkgConfig)
pkg_search_module(Eigen3 REQUIRED eigen3)
include_directories(${Eigen3_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(tests)
#add_subdirectory(examples)

install(DIRECTORY vpp DESTINATION include
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY vpp DESTINATION include
  FILES_MATCHING PATTERN "*.hpp")
