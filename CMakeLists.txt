cmake_minimum_required(VERSION 2.8)

project(ObserverPattern)

get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_BINARY_DIR}" NAME)
set(WSC_PROJECT ${CURRENT_DIR})

file(GLOB SRC "*.h" "*.hpp" ".c" "*.cpp" )


if(CMAKE_BUILD_TYPE MATCHES debug)
    message(STATUS "set CMAKE_CXX_FLAGS_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -O0 -fexceptions -std=c++11")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "set CMAKE_CXX_FLAGS_RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fexceptions")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNULINUX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt") # for boost
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread") # for boost
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_coroutine")

#add_executable(${WSC_PROJECTPROJECT} ${SRC})
add_executable(${PROJECT_NAME} "main.cpp")
