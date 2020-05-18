cmake_minimum_required(VERSION 3.0)
project(test)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        "${PROJECT_SOURCE_DIR}/include"
)

FILE(GLOB_RECURSE HEADER_LIST
#        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include/base/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/loadbalance/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/netpoll/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

add_executable(test a.cpp ${HEADER_LIST})