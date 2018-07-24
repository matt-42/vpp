cmake_minimum_required (VERSION 2.8)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_CURRENT_LIST_DIR}/cmake
                      ${CMAKE_SOURCE_DIR}/cmake)


find_package(OpenCV REQUIRED)

find_package(OpenGL REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${IOD_INCLUDE_DIR}
                    ${GLEW_INCLUDE_DIR}
         	    ${DIGE_INCLUDE_DIR})

set(LIBS
         ${OpenGL_LIBS}
	 ${OPENGL_gl_LIBRARY}
	 ${OPENGL_glu_LIBRARY}
         ${QT_LIBRARIES})

link_directories(${THIRDPARTY}/lib)

add_definitions(-std=c++1y -g)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -fopenmp -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -march=native")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(/usr/include/eigen3)

add_executable(box_filter box_filter.cc)
target_link_libraries(box_filter ${OpenCV_LIBS} gomp)

add_executable(fast_detector fast_detector.cc)
target_link_libraries(fast_detector ${OpenCV_LIBS} gomp)

add_executable(tutorial tutorial.cc)
target_link_libraries(tutorial ${OpenCV_LIBS} ${LIBS} gomp)


add_executable(video_extruder video_extruder.cc)
target_link_libraries(video_extruder ${OpenCV_LIBS} ${LIBS} gomp)


add_executable(video_extruder_paint video_extruder_paint.cc)
target_link_libraries(video_extruder_paint ${OpenCV_LIBS} ${LIBS} gomp)
