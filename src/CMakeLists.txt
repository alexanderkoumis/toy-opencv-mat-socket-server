cmake_minimum_required(VERSION 2.8.3)
project(pic_stream)

find_package(OpenCV 3 REQUIRED)

set(CMAKE_CXX_FLAGS "-gdwarf-3 -std=gnu++11")

add_executable(pic_client socket_client.cpp)
add_executable(pic_server socket_server.cpp)

include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(pic_client ${OpenCV_LIBS})
target_link_libraries(pic_server ${OpenCV_LIBS})
