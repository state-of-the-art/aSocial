cmake_minimum_required(VERSION 3.13)

project(asocial-crypto LANGUAGES CXX)

include(../../common.cmake)

find_package(Qt5 COMPONENTS Core REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Library_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Library_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")

include(libs/simpleqtcryptor.cmake)

add_library(${PROJECT_NAME} SHARED ${Library_CPP} ${Library_H})

target_link_directories(${PROJECT_NAME} PUBLIC ${LIBS_LIB_DIRS_${BUILD_ABI}})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core libcrypto.so libssl.so)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/simpleqtcryptor" ${LIBS_INCLUDE_DIRS_${BUILD_ABI}})

# Add the other modules include paths
target_include_directories(${PROJECT_NAME} PUBLIC ${MODULES_INCLUDE_DIRS})
