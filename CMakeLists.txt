# Compulsory
cmake_minimum_required(VERSION 3.20)

# Define project here
project(hello_world
        VERSION      1.2
        DESCRIPTION  "Hello World"
        HOMEPAGE_URL "https://docs.wxwidgets.org/3.0/overview_helloworld.html"
        LANGUAGES    C CXX) # Enable C language for third party libraries

# Activate verbose mode
#set(CMAKE_VERBOSE_MAKEFILE ON)
# Debug find package instruction
#set(CMAKE_FIND_DEBUG_MODE TRUE)

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

# FINDING DEPENDENCIES
#############################################################

# Find wxWidgets
find_package(wxWidgets 3.0 REQUIRED gl core base adv)
include(${wxWidgets_USE_FILE})

# Find GLUT
find_package(GLUT REQUIRED)
if (MSVC AND GLUT_FOUND)
  # Patch freeglut library under windows (x64 is missing)
  cmake_path(APPEND GLUT_ROOT "lib" "x64" "freeglut.lib" OUTPUT_VARIABLE GLUT_LIBRARIES)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# BUILDING
#############################################################

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


# PREPARING
#############################################################

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


# PACKAGING
#############################################################
cmake_path(APPEND CMAKE_CURRENT_SOURCE_DIR LICENSE OUTPUT_VARIABLE LICENSE_FILE_PATH)
cmake_path(APPEND CMAKE_CURRENT_SOURCE_DIR README.md OUTPUT_VARIABLE README_FILE_PATH)
# Package folder
cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR ${PROJECT_NAME} OUTPUT_VARIABLE PROJECT_FILE_PATH)
if (MSVC)
  # Archive file path
  set(ARCHIVE_FILE_NAME ${PROJECT_NAME}_${PROJECT_VERSION}_win64.zip)
  cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR $<CONFIG> ${ARCHIVE_FILE_NAME} OUTPUT_VARIABLE ARCHIVE_FILE_PATH)

  # Need to copy C++ freeglut shared library to binary directory
  add_custom_target(package_gui
    COMMAND ${CMAKE_COMMAND} -E echo "Creating ${ARCHIVE_FILE_PATH}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_FILE_PATH}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:hello> ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LICENSE_FILE_PATH} ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${README_FILE_PATH}  ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLUT_SHARED} ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E tar "c" ${ARCHIVE_FILE_PATH} --format=zip -- ${PROJECT_NAME}
  )
else()
  # Archive file path
  set(ARCHIVE_FILE_NAME ${PROJECT_NAME}_${PROJECT_VERSION}_linux64.tar.gz)
  cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR $<CONFIG> ${ARCHIVE_FILE_NAME} OUTPUT_VARIABLE ARCHIVE_FILE_PATH)
  
  add_custom_target(package_gui
    COMMAND ${CMAKE_COMMAND} -E echo "Creating ${ARCHIVE_FILE_PATH}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_FILE_PATH}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:hello> ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LICENSE_FILE_PATH} ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${README_FILE_PATH}  ${PROJECT_FILE_PATH}
    COMMAND ${CMAKE_COMMAND} -E tar "cz" ${ARCHIVE_FILE_PATH} -- ${PROJECT_NAME}
  )
endif()

# Compile executable before packaging
add_dependencies(package_gui hello)
