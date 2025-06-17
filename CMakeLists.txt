cmake_minimum_required(VERSION 3.10)
project(caplang)

set(CMAKE_CXX_STANDARD 17)

file(GLOB Sources src/*.cc)
add_library(caplang SHARED ${Sources})
target_include_directories(caplang PUBLIC include)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	target_compile_definitions(caplang PUBLIC CAP_DEBUG)
endif()

if(MSVC)
  target_compile_options(caplang PRIVATE /W4 /WX)
else()
  target_compile_options(caplang PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(capsandbox sandbox.cc)
target_link_libraries(capsandbox caplang)
