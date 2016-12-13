cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fopenmp=libomp -march=native")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..
  /usr/include/eigen3
  $ENV{HOME}/local/include)

add_executable(KITTI KITTI.cc)
target_link_libraries(KITTI ${OpenCV_LIBS})
