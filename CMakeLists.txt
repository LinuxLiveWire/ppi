cmake_minimum_required(VERSION 3.4)
project(ppi)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(ppi ${SOURCE_FILES})

target_link_libraries(ppi Qt5::Widgets)
