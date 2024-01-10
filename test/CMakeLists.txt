cmake_minimum_required(VERSION 3.5)
project(captests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(../cap cap)

file(GLOB Sources *.cc)
add_executable(captests ${Sources})

target_link_libraries(captests gtest_main cap)
target_include_directories(captests PUBLIC include)
