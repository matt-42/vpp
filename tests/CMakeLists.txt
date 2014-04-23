
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(imageNd imageNd.cc)
add_test(imageNd imageNd)
