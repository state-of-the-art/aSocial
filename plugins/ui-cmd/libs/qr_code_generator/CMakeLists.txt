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

project(qr_code_generator LANGUAGES CXX)

include(${PLUGINS_SOURCE_DIR}/../common.cmake)

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
