# Copyright (C) 2026  aSocial Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Author: Rabit (@rabits)

cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0135 NEW)

project(rocksdb)

include(${PLUGINS_SOURCE_DIR}/../common.cmake)

if(ANDROID)
  set(android_params
    -D ANDROID_ABI=${ANDROID_ABI}
    -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
    -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
  )
endif()

include(ExternalProject)
ExternalProject_Add(lib_${PROJECT_NAME}
  URL              ${lib_${PROJECT_NAME}_url}
  URL_HASH         ${lib_${PROJECT_NAME}_hash}
  DOWNLOAD_DIR     "${LIBS_DOWNLOAD_CACHE_DIR}/${PROJECT_NAME}"
  PREFIX           "${CMAKE_BINARY_DIR}/_build"
  INSTALL_DIR      "${LIBS_RESULT_DIR}"
  BUILD_ALWAYS     YES
  BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/librocksdb.a
  CMAKE_ARGS
    -D ROCKSDB_BUILD_SHARED=OFF
    -D WITH_GFLAGS=OFF
    -D WITH_TESTS=OFF
    -D WITH_BENCHMARK_TOOLS=OFF
    -D WITH_CORE_TOOLS=OFF
    -D WITH_TOOLS=OFF

    -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
    -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -D CMAKE_POSITION_INDEPENDENT_CODE=ON
    -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
    -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
    -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
    -D CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}

    ${android_params}
)
