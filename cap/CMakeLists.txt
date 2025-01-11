cmake_minimum_required(VERSION 3.10)
project(cap)

set(CMAKE_CXX_STANDARD 17)

file(GLOB Sources *.cc)
add_library(cap SHARED ${Sources})
target_include_directories(cap PUBLIC include)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	target_compile_definitions(cap PUBLIC CAP_DEBUG)
endif()
