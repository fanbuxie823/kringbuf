cmake_minimum_required(VERSION 3.0.0)
project(Demo VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O")

add_executable(Demo main.cpp)

target_link_libraries(Demo PRIVATE pthread)

target_compile_definitions(Demo PRIVATE $<$<CONFIG:Debug>:DEBUG_>)