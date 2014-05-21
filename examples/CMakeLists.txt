cmake_minimum_required (VERSION 2.6)

find_package(OpenCV REQUIRED)

add_definitions(-std=c++1y -g)
add_definitions(-Ofast -march=native -fopenmp)
add_definitions(-DNDEBUG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(/usr/include/eigen3)

add_executable(box_filter box_filter.cc)
target_link_libraries(box_filter ${OpenCV_LIBS} gomp)


add_executable(fast_detector fast_detector.cc)
target_link_libraries(fast_detector ${OpenCV_LIBS} gomp)
