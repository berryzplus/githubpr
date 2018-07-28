# CMakeList.txt : CMake project for GitHubPrTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# define a variable of project name
set(testName GitHubPrUnitTest)

# define a project name
project (${testName})

# define variables by file GLOB
file(GLOB testSources *.cpp)
file(GLOB testHeaders *.h)

# Add source to this project's executable.
add_executable(${testName} ${testHeaders} ${testSources})

# add definitions
target_compile_definitions(${testName} PUBLIC _CONSOLE UNICODE _UNICODE)

# add pre-compiled header to executable.
target_use_pre_compile_header(${testName} StdAfx.h StdAfx.cpp)

# add library modules
target_link_libraries(${testName} gtest gtest_main GitHubPRTestLib)
