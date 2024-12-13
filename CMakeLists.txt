cmake_minimum_required(VERSION 3.10)
project(CapSandbox)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(cap)
add_executable(capsandbox sandbox.cc)
target_link_libraries(capsandbox cap)
