cmake_minimum_required(VERSION 3.4)
project(ppi)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("winapp")

set(HEADER_FILES winapp/AppWindow.h winapp/TopWidget.h winapp/Ppi.h)
set(SOURCE_FILES winapp/AppWindow.cpp winapp/TopWidget.cpp winapp/Ppi.cpp)

add_executable(ppi main.cpp ${SOURCE_FILES})

target_link_libraries(ppi Qt5::Widgets)
