project (vpp_benchmarks)

include_directories(/usr/include/eigen3)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-Ofast -std=c++1y -march=native -DNDEBUG -fopenmp)

add_executable(boxNd_iterator boxNd_iterator.cc)
add_executable(iteration_on_domains iteration_on_domains.cc)
add_executable(image_iterations image_iterations.cc)
add_executable(image_add image_add.cc)
add_executable(box_filter box_filter.cc)

target_link_libraries(image_iterations gomp)
target_link_libraries(image_add gomp)
target_link_libraries(box_filter gomp)
