cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0135 NEW)

project(sodium LANGUAGES C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../common.cmake)

include(ProcessorCount)
ProcessorCount(CPU_N)

include(ExternalProject)
ExternalProject_Add(lib_${PROJECT_NAME}
  URL               ${lib_${PROJECT_NAME}_url}
  URL_HASH          ${lib_${PROJECT_NAME}_hash}
  DOWNLOAD_DIR      "${LIBS_DOWNLOAD_CACHE_DIR}/${PROJECT_NAME}"
  PREFIX            "${CMAKE_BINARY_DIR}/_build"
  INSTALL_DIR       "${LIBS_RESULT_DIR}"
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --disable-shared --prefix=<INSTALL_DIR>
  BUILD_COMMAND     make -j${CPU_N}
  INSTALL_COMMAND   make -j${CPU_N} install
  BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libsodium.a
  LOG_OUTPUT_ON_FAILURE true
)
