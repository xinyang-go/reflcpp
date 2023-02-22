cmake_minimum_required(VERSION 3.12)

project(reflcpp)

set(CMAKE_CXX_STANDARD 17)

add_library(reflcpp INTERFACE)
add_library(reflcpp::reflcpp ALIAS reflcpp)
target_include_directories(reflcpp INTERFACE include)

add_subdirectory(example)