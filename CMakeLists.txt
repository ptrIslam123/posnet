cmake_minimum_required(VERSION 3.18)

project(main)

add_subdirectory(core)

add_executable(main main.cpp)

target_link_libraries(main PUBLIC core)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)