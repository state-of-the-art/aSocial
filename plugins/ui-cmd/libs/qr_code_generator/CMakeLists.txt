cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0135 NEW)

project(qr_code_generator LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../common.cmake)

include(FetchContent)
FetchContent_Declare(
  lib_${PROJECT_NAME}
  URL           ${lib_${PROJECT_NAME}_url}
  URL_HASH      ${lib_${PROJECT_NAME}_hash}
  DOWNLOAD_DIR  "${LIBS_DOWNLOAD_CACHE_DIR}/${PROJECT_NAME}"
)
# Do not need to build the library - use only includes
FetchContent_MakeAvailable(lib_${PROJECT_NAME})

set(lib_${PROJECT_NAME}_INCLUDE_DIR ${lib_${PROJECT_NAME}_SOURCE_DIR}/cpp PARENT_SCOPE)

add_library(lib_${PROJECT_NAME} STATIC)
target_sources(lib_${PROJECT_NAME} PRIVATE ${lib_${PROJECT_NAME}_SOURCE_DIR}/cpp/qrcodegen.cpp)
target_include_directories(lib_${PROJECT_NAME} PRIVATE ${lib_${PROJECT_NAME}_INCLUDE_DIR})
