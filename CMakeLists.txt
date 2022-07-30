cmake_minimum_required(VERSION 3.0.0)
project(Demo VERSION 0.1.0)

add_executable(Demo main.cpp)

target_link_libraries(Demo PRIVATE pthread)

target_compile_definitions(Demo PRIVATE $<$<CONFIG:Debug>:DEBUG_>)