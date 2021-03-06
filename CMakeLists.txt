cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(PROJECT_NAME_STR pink-fluid)
project(${PROJECT_NAME_STR} C CXX)


#Set up the includes
include(ExternalProject)
include(CheckCXXCompilerFlag)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_CXX_DIR ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)
set(PROJECT_EXT_DIR ${PROJECT_SOURCE_DIR}/external)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
  message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
  message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


add_definitions(
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
  -DGLM_FORCE_RADIANS
)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(PROJECT_EXEC_FLAGS
    "-g -Wall -Wextra -pedantic"
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(PROJECT_EXEC_FLAGS
    "-g -Wall -Wextra -pedantic"
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

if(MSVC)
#vc 2012 fix for vararg templates
    set(MSVC_COMPILER_DEFS "-D_VARIADIC_MAX=10")
endif() 


find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
  message("Your compiler does not support OpenMP, performance will suffer.")
  message("Set your compiler via the CC and CXX environment variable to a compatible compiler.")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Options if you're using Clang is: http://clang-omp.github.io/ or use GCC")
  endif()
endif()


find_package(OpenGL REQUIRED)
find_package(Git REQUIRED)


if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	-lGL -lGLU -lXrandr -lXext -lX11 -lrt
	${CMAKE_DL_LIBS}
)
#Since there is no easy way to choose between _GLFW_HAS_GLXGETPROCADDRESS, _GLFW_HAS_GLXGETPROCADDRESSARB, _GLFW_HAS_GLXGETPROCADDRESSEXT and _GLFW_HAS_DLOPEN,
#I force the latter, which as least should exist (and work?) everywhere.
add_definitions(
	-D_GLFW_HAS_DLOPEN
	-D_GLFW_HAS_XRANDR
)
add_definitions(-D_UNIX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  SET(EXTRA_LIBS "-framework Cocoa -framework OpenGL -framework IOKit -framework Foundation -framework AppKit")
# TODO This generates compile errors from glfw on Mavericks, need to investigate further
# add_definitions(-D_MACOSX -ObjC++)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(GLEW_SOURCE
	${PROJECT_EXT_DIR}/glew/src/glew.c
)
add_library(GLEW STATIC
	${GLEW_SOURCE}
	${GLEW_INCLUDE}
)

target_link_libraries(GLEW
	${OPENGL_LIBRARY}
	${EXTRA_LIBS}
)


#We do not want the extra compiling that GLFW does
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Do not build Tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples")

# Get glfw
# This requies that submodules were initialized properly
add_subdirectory(
  ${PROJECT_EXT_DIR}/glfw/
)

#Get glm
set(glm_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/external/glm")
set(glm_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${glm_INSTALL_DIR} ")

externalProject_Add(glm
  SOURCE_DIR ${PROJECT_EXT_DIR}/glm
  CMAKE_ARGS -DENABLE_CPP11=NO ${glm_CMAKE_ARGS})

set(ext_libs 
  ${CMAKE_SOURCE_DIR}/external/glfw/include/
  ${CMAKE_SOURCE_DIR}/external/glew/include/
  ${PROJECT_EXT_DIR}/pcgsolver/
  ${PROJECT_EXT_DIR}/marchingcubes/
  ${glm_INSTALL_DIR}/include/
)

# Disables warnings under gcc and clang for GLM.
# GLM Emits a lot of warnings under higher compiler warnings
add_definitions(
  "-isystem ${glm_INSTALL_DIR}"
)

include_directories(
        ${ext_libs}
        ${PROJECT_INCLUDE_DIR}
)

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  glfw
  ${GLFW_LIBRARIES}
  GLEW
  INT_LIBS        
)

set(EXT_DEPS
    glm
    glfw
    googletest
    pthread
)


file(GLOB PROJECT_LIBS_FILES ${PROJECT_LIBS_DIR}/*.cpp)
add_library(INT_LIBS 
  ${PROJECT_LIBS_FILES}
)

add_dependencies(INT_LIBS
    ${EXT_DEPS}
)

#----------------------------
# Main Executable
#----------------------------

set(EXECUTABLE "${PROJECT_NAME_STR}")

add_executable(${EXECUTABLE}
  "${PROJECT_CXX_DIR}/${EXECUTABLE}.cpp"
)
  
set_target_properties(${EXECUTABLE}
  PROPERTIES COMPILE_FLAGS ${PROJECT_EXEC_FLAGS})  
  
add_dependencies(${EXECUTABLE} ${EXT_DEPS})

target_link_libraries(${EXECUTABLE}
  ${ALL_LIBS}
  )

#----------------------------
# Headless Executable
#----------------------------

set(HEADLESS_EXECUTABLE "${EXECUTABLE}-headless")

add_executable(${HEADLESS_EXECUTABLE}
  "${PROJECT_CXX_DIR}/${HEADLESS_EXECUTABLE}.cpp"
)
  
set_target_properties(${HEADLESS_EXECUTABLE}
  PROPERTIES COMPILE_FLAGS ${PROJECT_EXEC_FLAGS})  
  
add_dependencies(${HEADLESS_EXECUTABLE} ${EXT_DEPS})

target_link_libraries(${HEADLESS_EXECUTABLE}
  ${ALL_LIBS}
)

#----------------------------
# Maya Plugin
#----------------------------
find_package(Maya)

set(PROJECT_MAYA_NAME ${PROJECT_NAME_STR}_maya.bundle)
include_directories(${COMMON_INCLUDES})

file(GLOB MAYA_SRC_FILES ${PROJECT_SOURCE_DIR}/maya/main.cpp)
add_library(${PROJECT_MAYA_NAME} MODULE ${PROJECT_SOURCE_DIR}/maya/main.cpp)

add_definitions(-D_BOOL -DOSMac_ -DREQUIRE_IOSTREAM)
message(${MAYA_INCLUDE_DIRS})
include_directories(${MAYA_INCLUDE_DIRS})

set_target_properties(${PROJECT_MAYA_NAME}
  PROPERTIES COMPILE_FLAGS ${PROJECT_EXEC_FLAGS})

target_link_libraries(${PROJECT_MAYA_NAME}
                      ${ALL_LIBS}
                      "${MAYA_LIBRARIES}"
)

#If something breaks and if you are using mac, try this awesomeness:
#link_directories(/Applications/Autodesk/maya2015/Maya.app/Contents/MacOS)


#----------------------------
# Testing
#----------------------------
# Get gtest
add_subdirectory(
  ${PROJECT_EXT_DIR}/gtest
)
add_definitions(${MSVC_COMPILER_DEFS})
enable_testing()
set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)
include_directories(${GTEST_INCLUDE_DIRS} ${COMMON_INCLUDES})

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)
add_executable(${PROJECT_TEST_NAME} 
               ${TEST_SRC_FILES}
)
set_target_properties(${PROJECT_TEST_NAME}
  PROPERTIES COMPILE_FLAGS ${PROJECT_EXEC_FLAGS})
target_link_libraries(${PROJECT_TEST_NAME}
    ${ALL_LIBS}
)

# Create dependency of MainTest on googletest
add_dependencies(${PROJECT_TEST_NAME} ${EXT_DEPS})
if(MSVC)
  set(Suffix ".lib")
else()
  set(Suffix ".a")
  set(Pthread "-pthread")
endif()
target_link_libraries(${PROJECT_TEST_NAME}
       debug ${GTEST_LIBS_DIR}/DebugLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${Suffix}
       optimized ${GTEST_LIBS_DIR}/ReleaseLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${Suffix}
)

target_link_libraries(${PROJECT_TEST_NAME}
       debug ${GTEST_LIBS_DIR}/DebugLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${Suffix}
       optimized ${GTEST_LIBS_DIR}/ReleaseLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${Suffix}
       ${Pthread}
)

add_test(test ${PROJECT_TEST_NAME})



