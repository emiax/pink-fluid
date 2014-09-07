cmake_minimum_required(VERSION 2.8.8)
project(vtk_builder C CXX)
include(ExternalProject)

# Add gtest

set(vtk_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external/vtk")


ExternalProject_Add(vtk
    GIT_REPOSITORY https://github.com/Kitware/VTK.git
    PREFIX ${vtk_PREFIX}
    # Force separate output paths for debug and release builds to allow easy
    # identification of correct lib in subsequent TARGET_LINK_LIBRARIES commands
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -${gtest_CMAKE_ARGS}
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
    BUILD_IN_SOURCE 1

    # Disable install step
    INSTALL_COMMAND "")

# Specify include dir
set(VTK_INCLUDE_DIR ${vtk_PREFIX}/src PARENT_SCOPE)
include_directories(${VTK_INCLUDE_DIR})

# Specify MainTest's link libraries
ExternalProject_Get_Property(vtk binary_dir)
set(VTK_LIBS_DIR ${binary_dir} PARENT_SCOPE)
message("TROLLOLLO: ${VTK_INCLUDE_DIRS}")
# Add compiler flag for MSVC 2012
if(MSVC_VERSION EQUAL 1700)
  add_definitions(-D_VARIADIC_MAX=10)
endif()