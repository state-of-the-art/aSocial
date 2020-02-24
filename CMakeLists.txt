cmake_minimum_required(VERSION 3.13)

project(asocial VERSION 0.1.0)

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
include(common.cmake)

# Add targets to check the source code format
set(CLANG_FORMAT_BIN_NAME clang-format-10)
set(CLANG_FORMAT_EXCLUDE_PATTERNS "build/" "/libs/" ${CMAKE_BINARY_DIR})
find_package(ClangFormat)

# Build common libs
include(libs/libs.cmake)

# Modules build
include(modules/modules.cmake)

# Make sure the source code is well formatted
add_dependencies(${PROJECT_NAME}-${BUILD_MAIN_MODULE} format-check)

# TODO: Packaging
if(ANDROID)
  list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
  string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
  set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")
  message("Additional android libs: ${ANDROID_EXTRA_LIBS}")

  #add_custom_command(TARGET ${PROJECT_NAME}-${BUILD_MAIN_MODULE} POST_BUILD
  #  COMMAND sh "${CMAKE_SOURCE_DIR}/cmake/scripts/build-apk.sh" $<TARGET_FILE:${PROJECT_NAME}> ${ANDROID_EXTRA_LIBS} ${ANDROID_PACKAGE_SOURCE_DIR})
endif()
