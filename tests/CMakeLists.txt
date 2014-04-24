
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

add_executable(parallel_for parallel_for.cc)
add_test(parallel_for parallel_for)
