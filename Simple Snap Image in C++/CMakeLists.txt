project(simple-snapimage)
 
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

# Enter the directory of the tiscamera repository here:
set( TISCAMERA_DIR /home/bvtest/projects/tiscamera-1.0) 

set(CMAKE_BUILD_TYPE Debug)

message( ${TISCAMERA_DIR} )

if(NOT IS_DIRECTORY "${TISCAMERA_DIR}/examples/cpp/common" )
    message( "FATAL ERROR : TISCAMERA_DIR not set to tiscamera directory." )
    return()
endif()

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(TCAMLIB tcam)


include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${TISCAMERA_DIR}/examples/cpp/common  ${GSTREAMER_INCLUDE_DIRS} ${TCAM_INCLUDE_DIRS}  ${OpenCV_INCLUDE_DIRS} )

add_definitions(${GSTREAMER_CFLAGS_OTHER})  

add_executable(simple-snapimage main.cpp tcamimage.cpp ${TISCAMERA_DIR}/examples/cpp/common/tcamcamera.cpp )
target_link_libraries(simple-snapimage ${TCAMLIB_LIBRARIES} ${GSTREAMER_LIBRARIES}  ${OpenCV_LIBS})


install(TARGETS simple-snapimage RUNTIME DESTINATION bin)
