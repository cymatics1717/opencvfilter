cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 11)
set (OPENCV_REQUIRED 3.0.0)
set (GST_REQUIRED 1.8)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

find_package(PkgConfig)
project(gstcalib)

#pkg_check_modules(OPENCV REQUIRED opencv>=${OPENCV_REQUIRED})
pkg_check_modules (GSTREAMER gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0)

find_package( OpenCV REQUIRED )

set(OPENCV_FILTER_SOURCES
    cvplugin.c
    cvfilter.cpp cvfilter.h
    )


add_library(${PROJECT_NAME} MODULE ${OPENCV_FILTER_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER-BASE_LIBRARIES}
    ${GSTREAMER-VIDEO_LIBRARIES}
    ${OpenCV_LIBS}
    pthread
    )
