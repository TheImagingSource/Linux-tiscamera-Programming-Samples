project(qt5-dialogs)
 
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)

# Enter the directory of the tiscamera repository here:
set( TISCAMERA_DIR /home/bvtest/projects/tiscamera-1.0) 

set(CMAKE_BUILD_TYPE Debug)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(TCAMLIB tcam)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core)

if (Qt5Widgets_FOUND)
    if (Qt5Widgets_VERSION VERSION_LESS 5.5)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.5" ${Qt5_DIR} ${QT_QMAKE_EXECUTABLE})
    endif()
else()
    message(SEND_ERROR "The Qt5Widgets library could not be found!")
endif(Qt5Widgets_FOUND)



include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${TISCAMERA_DIR}/examples/cpp/common  ${GSTREAMER_INCLUDE_DIRS} ${TCAM_INCLUDE_DIRS})

add_definitions(${GSTREAMER_CFLAGS_OTHER})  

add_executable(qt5-dialogs main.cpp mainwindow.cpp cdeviceselectiondlg.cpp cpropertiesdialog.cpp tcamcamera.cpp )
target_link_libraries(qt5-dialogs ${TCAMLIB_LIBRARIES} ${GSTREAMER_LIBRARIES} Qt5::Widgets Qt5::Core)


install(TARGETS qt5-dialogs RUNTIME DESTINATION bin)
