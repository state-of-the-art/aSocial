cmake_minimum_required(VERSION 3.5)

project(${PROJECT_NAME}-crypto LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Library_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Library_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")

include(lib/simpleqtcryptor.cmake)

add_library(${PROJECT_NAME} SHARED ${Library_CPP} ${Library_H})

include(../../lib/openssl.cmake)

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core ExOpenSSL::Crypto ExOpenSSL::SSL)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/simpleqtcryptor" "${EX_OPENSSL_INCLUDE_DIR}")
