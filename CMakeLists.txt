cmake_minimum_required(VERSION 3.11)

project(cmake-tree LANGUAGES CXX)

add_subdirectory(3rd)

add_executable(cmake-tree src/main.cc)
target_link_libraries(cmake-tree PRIVATE screen dom component subprocess)

