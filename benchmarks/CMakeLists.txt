project (vpp_benchmarks)

include_directories(/usr/include/eigen3)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-Ofast -std=c++1y -march=native -DNDEBUG -fopenmp -g)

add_executable(boxNd_iterator boxNd_iterator.cc)
add_executable(iteration_on_domains iteration_on_domains.cc)
add_executable(image_iterator image_iterator.cc)
add_executable(parallel_for parallel_for.cc)

target_link_libraries(parallel_for gomp)