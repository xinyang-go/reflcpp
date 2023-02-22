find_package(yaml-cpp)
find_package(Boost COMPONENTS serialization)

add_executable(reflcpp_example example.cpp)
target_include_directories(reflcpp_example PRIVATE ${YAML_CPP_INCLUDE_DIR})
target_link_libraries(reflcpp_example PRIVATE reflcpp::reflcpp Boost::serialization ${YAML_CPP_LIBRARIES})
target_compile_options(reflcpp_example PRIVATE -Wall)