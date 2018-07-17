# CMakeList.txt : CMake project for GitHubPR, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# define a variable of project name
set(projectName GitHubPR)

# define a project name
project (${projectName})

# set the project as the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${projectName})


# define variables by file GLOB
file(GLOB_RECURSE cppSources ./src/*.cpp)
file(GLOB_RECURSE cppHeaders ./include/*.h)
file(GLOB_RECURSE resourceScripts ./src/*.rc)

# Add source to this project's executable.
add_executable(${projectName} ${cppSources} ${cppHeaders} ${resourceScripts})

# set include directories
include_directories(./include)

# setup pre-compiled header
set(PreCompiledHeader StdAfx.h)
set(PreCompiledSource ./src/StdAfx.cpp)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  add_definitions(/Yu"${PreCompiledHeader}")
  set_source_files_properties(${PreCompiledSource} PPROPERTIES COMPILE_FLAGS /Yc"${PreCompiledHeader}")
endif(CMAKE_GENERATOR MATCHES "Visual Studio")

# add definitions
add_definitions(-DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE)

# add library modules
target_link_libraries(${projectName} Shlwapi.lib)

# TODO: Add tests and install targets if needed.
