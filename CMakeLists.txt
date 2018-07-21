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
file(GLOB_RECURSE cppSources src/*.cpp)
file(GLOB_RECURSE cppHeaders include/*.h)
file(GLOB_RECURSE resourceScripts src/*.rc)

# Add source to this project's executable.
add_executable(${projectName} ${cppSources} ${cppHeaders} ${resourceScripts})

# setup pre-compiled header
set(PreCompiledHeader StdAfx.h)
set(PreCompiledSource src/StdAfx.cpp)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_source_files_properties(${cppSources} PPROPERTIES COMPILE_FLAGS /Yu"${PreCompiledHeader}")
  set_source_files_properties(${PreCompiledSource} PPROPERTIES COMPILE_FLAGS /Yc"${PreCompiledHeader}")
endif(CMAKE_GENERATOR MATCHES "Visual Studio")

# add definitions
target_compile_definitions(${projectName} PUBLIC _CONSOLE UNICODE _UNICODE)

# add include directories
target_include_directories(${projectName} PRIVATE include)

# add library modules
target_link_libraries(${projectName} Shlwapi.lib)

# TODO: Add tests and install targets if needed.
