
find_package(OpenCV REQUIRED)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

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

target_link_libraries(opencv_bridge ${OpenCV_LIBS})
