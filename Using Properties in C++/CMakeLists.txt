project(using-properties)
 
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)

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


include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${TISCAMERA_DIR}/examples/cpp/common  ${GSTREAMER_INCLUDE_DIRS} ${TCAM_INCLUDE_DIRS} )
add_definitions(${GSTREAMER_CFLAGS_OTHER})  

add_executable(using-properties main.cpp ${TISCAMERA_DIR}/examples/cpp/common/tcamcamera.cpp )

target_link_libraries(using-properties ${TCAMLIB_LIBRARIES} ${GSTREAMER_LIBRARIES} )

install(TARGETS using-properties RUNTIME DESTINATION bin)
