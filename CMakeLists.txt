cmake_minimum_required(VERSION 3.16)

project(asocial LANGUAGES CXX VERSION 0.1.0)

# Common parameters
include(common.cmake)

# Build plugins
include(plugins.cmake)

find_package(Qt6 COMPONENTS Core LinguistTools REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")
file(GLOB_RECURSE Execution_TS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "i18n/*.ts")

qt_add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${Execution_TS})
