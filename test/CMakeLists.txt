# CMakeList.txt : CMake project for GitHubPrTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# try to find google test
# https://cmake.org/cmake/help/v3.11/module/FindGTest.html
find_package(GTest QUIET)

if (NOT GTEST_FOUND)
  message(FATAL ": google test not found.")

  # add tests
  add_subdirectory(googletest)

  # find google test(REQUIRED)
  #find_package(GTest REQUIRED)

endif (NOT GTEST_FOUND)
