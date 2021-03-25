project(tcam-dialog)
 
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

#add_subdirectory( lib )

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(TCAMLIB tcam)


include_directories( ${CMAKE_CURRENT_BINARY_DIR} 
                    ${GSTREAMER_INCLUDE_DIRS} 
                    ${TCAM_INCLUDE_DIRS}  
                    ${Threads_INCLUDE_DIRS} 
                    ${Curses_INCLUDE_DIRS} 
                    ${FMT_INCLUDE_DIRS} 
                    )
                    
add_definitions(${GSTREAMER_CFLAGS_OTHER})  

add_executable(tcam-dialog 
                main.cpp 
                camera/camerabase.cpp
                controls/cursesprogressbar.cpp
                controls/win.cpp
                )

target_link_libraries(tcam-dialog 
                        ${TCAMLIB_LIBRARIES}
                        ${GSTREAMER_LIBRARIES} 
                        ${CURSES_LIBRARIES}  
                        Threads::Threads 
                    )



 