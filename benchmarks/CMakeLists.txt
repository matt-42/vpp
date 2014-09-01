cmake_minimum_required(VERSION 2.8)
project (vpp_benchmarks)

find_package(OpenCV REQUIRED)

include_directories(/usr/include/eigen3)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-O3 -std=c++1y -march=native -DNDEBUG -fopenmp -ftree-vectorizer-verbose=5 -Wopenmp-simd)

set(CMAKE_VERBOSE_MAKEFILE ON)
add_executable(boxNd_iterator boxNd_iterator.cc)
add_executable(iteration_on_domains iteration_on_domains.cc)
add_executable(image_iterations image_iterations.cc)
add_executable(image_add image_add.cc)
add_executable(box_filter box_filter.cc)
add_executable(fast_detector fast_detector.cc)
add_executable(for_fast for_fast.cc)
add_executable(pyrlk_opencv_comparison pyrlk_opencv_comparison.cc)

target_link_libraries(image_iterations gomp)
target_link_libraries(image_add gomp)
target_link_libraries(box_filter gomp)
target_link_libraries(fast_detector ${OpenCV_LIBS}  gomp)
target_link_libraries(for_fast ${OpenCV_LIBS}  gomp)
target_link_libraries(pyrlk_opencv_comparison ${OpenCV_LIBS} gomp)
