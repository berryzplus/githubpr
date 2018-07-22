# CMakeList.txt : CMake project for GitHubPR, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# setup pre-compiled header
function(target_use_pre_compile_header projectName PreCompiledHeader PreCompiledHeaderSource)
  if (CMAKE_GENERATOR MATCHES "Visual Studio")
    target_compile_options(${projectName} PRIVATE /Yu"${PreCompiledHeader}")
	set_source_files_properties(${PreCompiledHeaderSource}
      PROPERTIES
        COMPILE_FLAGS /Yc"${PreCompiledHeader}"
    )
  endif(CMAKE_GENERATOR MATCHES "Visual Studio")
endfunction(target_use_pre_compile_header)


# define a variable of project name
set(projectName GitHubPR)

# define a project name
project (${projectName})

# set the project as the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${projectName})

# define variables by file GLOB
file(GLOB_RECURSE cppHeaders include/*.h)
file(GLOB_RECURSE cppSources src/*.cpp)
file(GLOB_RECURSE resourceScripts src/*.rc)

# Add source to this project's executable.
add_executable(${projectName} ${cppHeaders} ${cppSources} ${resourceScripts})

# add definitions
target_compile_definitions(${projectName} PUBLIC _CONSOLE UNICODE _UNICODE)

# add include directories
target_include_directories(${projectName} PRIVATE include)

# add pre-compiled header to executable.
target_use_pre_compile_header(${projectName} StdAfx.h src/StdAfx.cpp)

# add library modules
target_link_libraries(${projectName} Shlwapi.lib)

# TODO: Add tests and install targets if needed.
