cmake_minimum_required(VERSION 2.8)
project(toy-opencv-sockets)

find_package(OpenCV 3 REQUIRED)

set(CMAKE_CXX_FLAGS "-std=gnu++11")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ggdb -gdwarf-3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OpenCV_INCLUDE_DIRS}
)

add_executable(mat_client_server
    sample/main.cpp
    sample/helpers.cpp
    socket-client.cpp
    socket-server.cpp
)

target_link_libraries(mat_client_server
    ${OpenCV_LIBS}
    pthread
)
