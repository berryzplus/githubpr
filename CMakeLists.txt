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
function(make_pre_compiled_header _preCompiledHeaderName _preCompiledSourceName _cppSources)
  set(PreCompiledHeader _preCompiledHeaderName)
  if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(PreCompiledSource _preCompiledSourceName)
    set_source_files_properties(_cppSources PPROPERTIES COMPILE_FLAGS /Yu"${PreCompiledHeader}")
    set_source_files_properties(${PreCompiledSource} PPROPERTIES COMPILE_FLAGS /Yc"${PreCompiledHeader}")
  endif(CMAKE_GENERATOR MATCHES "Visual Studio")
endfunction(make_pre_compiled_header)

# add definitions
target_compile_definitions(${projectName} PUBLIC _CONSOLE UNICODE _UNICODE)

# add include directories
target_include_directories(${projectName} PRIVATE include)

# Add source to this project's executable.
make_pre_compiled_header(StdAfx.h src/StdAfx.cpp ${cppSources})

# add library modules
target_link_libraries(${projectName} Shlwapi.lib)

# TODO: Add tests and install targets if needed.
