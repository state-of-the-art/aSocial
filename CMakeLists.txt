cmake_minimum_required(VERSION 3.5)

project(asocial LANGUAGES CXX VERSION 0.1.0)

if(ANDROID)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(ANDROID_STL "c++_shared" CACHE STRING "Force to prevent issues" FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Options & user parameters
set(BUILD_MAIN_MODULE "frontend" CACHE STRING "The main module to build")
set(BUILD_EXCLUDE_MODULES "" CACHE STRING "List of modules to exclude, separated by ';'")

# System parameters
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Add targets to check the source code format
set(CLANG_FORMAT_BIN_NAME clang-format-10)
set(CLANG_FORMAT_EXCLUDE_PATTERNS "build/" "/lib/" ${CMAKE_BINARY_DIR})
find_package(ClangFormat)

# Modules build
add_subdirectory(modules)

# Make sure the source code is well formatted
add_dependencies("${PROJECT_NAME}" format-check)
