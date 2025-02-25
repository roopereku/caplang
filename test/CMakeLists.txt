cmake_minimum_required(VERSION 3.14)
project(CapTests)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(../cap cap)

file(GLOB Sources *.cc)
add_executable(captests ${Sources})
target_link_libraries(captests cap GTest::gtest_main)
target_include_directories(captests PRIVATE include)

if(MSVC)
  target_compile_options(captests PRIVATE /W4 /WX)
else()
  target_compile_options(captests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(GoogleTest)
gtest_discover_tests(captests)
