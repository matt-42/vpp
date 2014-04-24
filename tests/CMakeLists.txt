
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(imageNd imageNd.cc)
add_test(imageNd imageNd)


add_executable(image2d image2d.cc)
add_test(image2d image2d)
