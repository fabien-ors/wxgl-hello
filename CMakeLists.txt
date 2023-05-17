# Compulsory
cmake_minimum_required(VERSION 3.20)

# Define project here
project(hello_world
        VERSION      1.1
        DESCRIPTION  "Hello World"
        HOMEPAGE_URL "https://docs.wxwidgets.org/3.0/overview_helloworld.html"
        LANGUAGES    C CXX) # Enable C language for third party libraries

# Activate verbose mode
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Detect presence of multi configuration generators
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Make Release version the default (only for single configuration generators)
if(NOT CMAKE_BUILD_TYPE AND NOT IS_MULTI_CONFIG)
  message(STATUS "Setting build type to 'Release' as none was specified")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Generation folder (into Release or Debug)
if (NOT IS_MULTI_CONFIG)
  cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR ${CMAKE_BUILD_TYPE} OUTPUT_VARIABLE CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR ${CMAKE_BUILD_TYPE} OUTPUT_VARIABLE CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR ${CMAKE_BUILD_TYPE} OUTPUT_VARIABLE CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
endif()

# Add c++11 support whatever the compiler
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find wxWidgets
find_package(wxWidgets 3.0 REQUIRED gl core base adv)
include(${wxWidgets_USE_FILE})

# Find GLUT
find_package(GLUT REQUIRED)
if (MSVC AND GLUT_FOUND)
  # Patch freeglut library under windows (x64 is missing)
  cmake_path(APPEND GLUT_ROOT "lib" "x64" "freeglut.lib" OUTPUT_VARIABLE GLUT_LIBRARIES)
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# Add executable target
add_executable(hello main.cpp)

# Include directories
target_include_directories(hello PRIVATE
  # GLUT include dir
  $<BUILD_INTERFACE: ${GLUT_INCLUDE_DIR}>
  # OpenGL include dir
  $<BUILD_INTERFACE: ${OpenGL_INCLUDE_DIRS}>
)

# Link executable to wxWidgets library
target_link_libraries(hello PRIVATE ${wxWidgets_LIBRARIES})
# Link to OpenGL and FreeGLUT
target_link_libraries(hello PRIVATE ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

if (MSVC)
  # Need to copy C++ freeglut shared library to binary directory
  cmake_path(APPEND GLUT_ROOT "bin" "x64" "freeglut.dll" OUTPUT_VARIABLE GLUT_SHARED)
  cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR $<CONFIG> OUTPUT_VARIABLE RUNTIME_DIR)
  
  add_custom_target(prepare_hello
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RUNTIME_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLUT_SHARED} ${RUNTIME_DIR}
  )
  # Trigger the prepare target each time executable is compiled
  add_dependencies(hello prepare_hello)
endif()


