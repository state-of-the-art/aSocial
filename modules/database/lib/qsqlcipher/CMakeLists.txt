cmake_minimum_required(VERSION 3.5)

project(qsqlcipher LANGUAGES CXX VERSION 0.1.0)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

include(../../../../lib/openssl.cmake)
include(ExternalProject)

set(SQLCIPHER_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/sqlcipher/lib")

set(sqlcipher_opts --enable-tempstore=yes
    CFLAGS=-DSQLITE_HAS_CODEC\ -fPIC\ -I${EX_OPENSSL_INCLUDE_DIR}
    LDFLAGS=-L${EX_OPENSSL_LIB_DIR}\ -lcrypto)

ExternalProject_Add(sqlcipher
  DEPENDS           ex_openssl
  PREFIX            ${CMAKE_CURRENT_BINARY_DIR}
  URL               https://github.com/sqlcipher/sqlcipher/archive/v${SQLCIPHER_VERSION}.tar.gz
  URL_HASH          SHA1=${SQLCIPHER_HASH}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared=no ${sqlcipher_opts} --prefix=${CMAKE_CURRENT_BINARY_DIR}/sqlcipher
  BUILD_COMMAND     make -j${CPU_N}
  INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/sqlcipher
  BUILD_BYPRODUCTS  ${SQLCIPHER_LIB_DIR}/libsqlcipher.a
)

find_package(Qt5 COMPONENTS Core Sql REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Library_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
file(GLOB_RECURSE Library_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(qsqlcipher STATIC ${Library_CPP} ${Library_H})

add_dependencies(qsqlcipher sqlcipher)
target_compile_definitions(qsqlcipher PRIVATE SQLITE_HAS_CODEC)
target_link_libraries(qsqlcipher PRIVATE Qt5::Core Qt5::Sql)
target_link_libraries(qsqlcipher PRIVATE Qt5::SqlPrivate ${SQLCIPHER_LIB_DIR}/libsqlcipher.a)
target_include_directories(qsqlcipher PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/sqlcipher/include")
