add_executable(reflcpp_example example.cpp)
target_link_libraries(reflcpp_example reflcpp::reflcpp)
target_compile_options(reflcpp_example PRIVATE -Wall)