cmake_minimum_required(VERSION 3.5)

project(qsqlcipher LANGUAGES CXX C VERSION 0.1.0)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

find_package(Qt5 COMPONENTS Core Sql REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Library_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
file(GLOB_RECURSE Library_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include(sqlcipher_generate_src.cmake)

add_library(qsqlcipher STATIC ${Library_CPP} ${Library_H}
  ${SQLCIPHER_INCLUDE_DIR}/sqlcipher/sqlite3.h
  ${SQLCIPHER_GEN_DIR}/sqlite3.c
)

add_dependencies(qsqlcipher sqlcipher)
target_compile_definitions(qsqlcipher PRIVATE SQLITE_HAS_CODEC)
target_link_libraries(qsqlcipher PUBLIC Qt5::Core Qt5::Sql PRIVATE Qt5::SqlPrivate)
target_include_directories(qsqlcipher PRIVATE ${SQLCIPHER_INCLUDE_DIR} ${LIBS_INCLUDE_DIRS_${BUILD_ABI}})
