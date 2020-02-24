cmake_minimum_required(VERSION 3.13)

project(asocial-backend LANGUAGES CXX)

include(../../common.cmake)

find_package(Qt5 COMPONENTS Core REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Library_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Library_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")

add_library(${PROJECT_NAME} SHARED ${Library_CPP} ${Library_H})

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core)

# Add the other modules include paths
target_include_directories(${PROJECT_NAME} PUBLIC ${MODULES_INCLUDE_DIRS})
