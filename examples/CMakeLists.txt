cmake_minimum_required (VERSION 2.8)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_CURRENT_LIST_DIR}/cmake
                      ${CMAKE_SOURCE_DIR}/cmake)


find_package(OpenCV REQUIRED)

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
find_package(OpenGL REQUIRED)

include(${QT_USE_FILE})

SET(THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/install)

include_directories(${THIRDPARTY}/include
                    ${GLEW_INCLUDE_DIR}
         	    ${DIGE_INCLUDE_DIR})

set(LIBS
         dige
         ${OpenGL_LIBS}
	 ${OPENGL_gl_LIBRARY}
	 ${OPENGL_glu_LIBRARY}
         ${QT_LIBRARIES})

link_directories(${THIRDPARTY}/lib)

add_definitions(-std=c++1y -g -fopenmp)
add_definitions(-Ofast -march=native )
add_definitions(-DNDEBUG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(/usr/include/eigen3)

add_executable(box_filter box_filter.cc)
target_link_libraries(box_filter ${OpenCV_LIBS} gomp)

add_executable(fast_detector fast_detector.cc)
target_link_libraries(fast_detector ${OpenCV_LIBS} gomp)

add_executable(pyrlk pyrlk.cc)
target_link_libraries(pyrlk ${OpenCV_LIBS} ${LIBS} gomp)
