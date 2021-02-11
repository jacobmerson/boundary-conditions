cmake_minimum_required(VERSION 3.13.4)
project(BoundaryConditions VERSION 0.0.1 LANGUAGES CXX)
include(CTest)

# Set the language standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic")
endif()


# RPATHS stuff...this is important to make spack happy in all situations
# enable @rpath in the install name for any shared library being built
# note: it is planned that a future version of CMake will enable this by default
set(CMAKE_MACOSX_RPATH 1)
# Always use full RPATH
# http://www.cmake.org/Wiki/CMake_RPATH_handling
# http://www.kitware.com/blog/home/post/510
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")



add_subdirectory(tpl)
add_subdirectory(src)
add_subdirectory(exe)

if(BUILD_TESTING)
  include(CTest)
  find_package(Catch2 REQUIRED)
  add_subdirectory(test)
endif()
