cmake_minimum_required(VERSION 3.15)
project(isaserver)

set(CMAKE_CXX_STANDARD 11)

add_executable(isaserver isaserver.cpp isaserver.h)
find_package (Threads)
target_link_libraries (isaserver ${CMAKE_THREAD_LIBS_INIT})
