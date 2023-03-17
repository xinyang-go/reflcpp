cmake_minimum_required(VERSION 3.12)

project(reflcpp)

set(CMAKE_CXX_STANDARD 17)

add_library(reflcpp INTERFACE)
add_library(reflcpp::reflcpp ALIAS reflcpp)
target_include_directories(reflcpp INTERFACE include)

option(REFLCPP_BUILD_EXAMPLE "reflcpp example" OFF)
if(REFLCPP_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()