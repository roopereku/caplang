cmake_minimum_required(VERSION 3.0)
project(caplang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# TODO The user should have to enable debug mode
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DDEBUG)

FILE(GLOB CapSources cap/*.cc cap/arch/*.cc)

# Create the akimbo library and link it with necessary stuff
add_library(caplang ${CapSources})

# Create a testbed for akimbo
add_executable(captest test.cc)
add_dependencies(captest caplang)
target_link_libraries(captest caplang)
