cmake_minimum_required(VERSION 3.28)
project(ChunkedList)

set(CMAKE_CXX_STANDARD 17)

add_library(ChunkedList STATIC ChunkedList.hpp
        ChunkedList.cpp)
