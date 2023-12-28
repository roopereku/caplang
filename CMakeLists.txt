cmake_minimum_required(VERSION 3.5)
project(caplang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(cap)

add_executable(captest test.cc)
target_link_libraries(captest cap)
