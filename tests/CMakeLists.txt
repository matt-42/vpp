cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. /usr/include/eigen3 $ENV{HOME}/projects/iod)
add_definitions(-std=c++14 -g -fopenmp)

add_executable(imageNd imageNd.cc)
add_test(imageNd imageNd)

add_executable(image2d image2d.cc)
add_test(image2d image2d)

add_executable(image3d image3d.cc)
add_test(image3d image3d)

add_executable(boxNd_iterator boxNd_iterator.cc)
add_test(boxNd_iterator boxNd_iterator)

add_executable(imageNd_iterator imageNd_iterator.cc)
add_test(imageNd_iterator imageNd_iterator)

add_executable(parallel_for parallel_for.cc)
target_link_libraries(parallel_for gomp)
add_test(parallel_for parallel_for)

add_executable(box_nbh2d box_nbh2d.cc)
add_test(box_nbh2d box_nbh2d)

add_executable(sandbox sandbox.cc)
add_test(sandbox sandbox)

add_executable(window window.cc)
add_test(window window)

add_executable(opencv_bridge opencv_bridge.cc)
add_test(opencv_bridge opencv_bridge)

add_executable(fill fill.cc)
add_test(fill fill)

add_executable(pyramid pyramid.cc)
add_test(pyramid pyramid)

add_executable(cast cast.cc)
add_test(cast cast)

add_executable(sum sum.cc)
add_test(sum sum)

add_executable(tuple_utils tuple_utils.cc)
add_test(tuple_utils tuple_utils)

add_executable(block_wise block_wise.cc)
add_test(block_wise block_wise)


add_executable(colorspace_conversions colorspace_conversions.cc)
add_test(colorspace_conversions colorspace_conversions)

add_executable(pyrlk pyrlk.cc)
add_test(pyrlk pyrlk)

target_link_libraries(pyrlk ${OpenCV_LIBS})
target_link_libraries(opencv_bridge ${OpenCV_LIBS})
