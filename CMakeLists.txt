﻿# CMakeList.txt : CMake project for GitHubPR, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include(GNUInstallDirs)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # CMake compile with /MT instead of /MD
  # https://stackoverflow.com/questions/14172856/cmake-compile-with-mt-instead-of-md
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")


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

# turn on solution folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(${projectName}
    PROPERTIES LINK_FLAGS "-municode")
  set(CMAKE_RC_FLAGS "-c utf8")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

# create solution folder
set_target_properties(${projectName} PROPERTIES FOLDER executable)


# define a variable of project name
set(libName LibGitHubPR)

# define variables by file GLOB
file(GLOB_RECURSE libHeaders include/*.h)
file(GLOB_RECURSE libSources src/*.cpp)
#file(GLOB_RECURSE resourceScripts src/*.rc)

# exclude wmain from libSources
list(FILTER libSources EXCLUDE REGEX ".*wmain.cpp$")

# Add source to this project's executable.
add_library(${libName} STATIC ${libHeaders} ${libSources} ${resourceScripts})

# add definitions
target_compile_definitions(${libName} PUBLIC _CONSOLE UNICODE _UNICODE)

# add include directories
target_include_directories(${libName} PUBLIC include)

# add pre-compiled header to executable.
target_use_pre_compile_header(${libName} StdAfx.h src/StdAfx.cpp)

# add library modules
target_link_libraries(${libName} Shlwapi.lib)

# create solution folder
set_target_properties(${libName} PROPERTIES FOLDER unitTest)

# Install rules
install(TARGETS ${libName}
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY  "${SOURCE_DIR}/include"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# configure and install pkgconfig files
configure_file(
  cmake/${libName}.pc.in
  "${CMAKE_BINARY_DIR}/${libName}.pc"
  @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/${libName}.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")


# add google test
add_subdirectory(googletest)

# create solution folder
set_target_properties(gmock PROPERTIES FOLDER GoogleTest)
set_target_properties(gmock_main PROPERTIES FOLDER GoogleTest)
set_target_properties(gtest PROPERTIES FOLDER GoogleTest)
set_target_properties(gtest_main PROPERTIES FOLDER GoogleTest)

# add test directory
add_subdirectory(test)

# create solution folder
set_target_properties(GitHubPrUnitTest PROPERTIES FOLDER unitTest)


# enable testing
enable_testing()

# add test
add_test(NAME unitTest COMMAND GitHubPrUnitTest)
