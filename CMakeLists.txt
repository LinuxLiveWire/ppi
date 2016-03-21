cmake_minimum_required(VERSION 3.2)
project(ppi)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(DEFINED RADAR_TYPE)
    if(RADAR_TYPE STREQUAL P18)
        add_definitions(-DRADAR_TYPE=1)
        message(STATUS "RADAR_TYPE==P18")
    elseif(RADAR_TYPE STREQUAL P19)
        add_definitions(-DRADAR_TYPE=2)
        message(STATUS "RADAR_TYPE==P19")
    elseif(RADAR_TYPE STREQUAL TRLK10)
        add_definitions(-DRADAR_TYPE=3)
        message(STATUS "RADAR_TYPE==TRLK10")
    elseif(RADAR_TYPE STREQUAL P14)
        add_definitions(-DRADAR_TYPE=4)
        message(STATUS "RADAR_TYPE==P14")
    elseif(RADAR_TYPE STREQUAL EKRAN85)
        add_definitions(-DRADAR_TYPE=5)
        message(STATUS "RADAR_TYPE==EKRAN85")
    elseif(RADAR_TYPE STREQUAL RSP10)
        add_definitions(-DRADAR_TYPE=6)
        message(STATUS "RADAR_TYPE==RSP10")
    else(RADAR_TYPE STREQUAL P18)
        message(FATAL_ERROR "Undefined radar type - ${RADAR_TYPE}.
        Possible values - P18/P19/TRLK10/P14/EKRAN85/RSP10")
    endif(RADAR_TYPE STREQUAL P18)
else(DEFINED RADAR_TYPE)
    add_definitions(-DRADAR_TYPE=1)
    message(STATUS "RADAR_TYPE==18")
endif(DEFINED RADAR_TYPE)

include_directories("winapp")

set(HEADER_FILES winapp/AppWindow.h winapp/TopWidget.h winapp/Ppi.h)
set(SOURCE_FILES winapp/AppWindow.cpp winapp/TopWidget.cpp winapp/Ppi.cpp winapp/PpiControl.h winapp/PpiControl.cpp)

add_executable(ppi main.cpp ${SOURCE_FILES})

target_link_libraries(ppi Qt5::Widgets Qt5::OpenGL)
