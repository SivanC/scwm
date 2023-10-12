cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME scwm)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
# change -isystem to -I
set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE)
# where cmake looks for modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(SOURCE_FILES main.cpp window_manager.cpp window_manager.hpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(X11 REQUIRED)
find_package(glog REQUIRED)

target_link_libraries(${PROJECT_NAME} glog ${X11_LIBRARIES})

