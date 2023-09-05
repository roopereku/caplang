cmake_minimum_required(VERSION 3.5)
project(libcap)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

file(GLOB Sources *.cc)
add_library(libcap ${Sources})
target_include_directories(libcap PUBLIC include)
