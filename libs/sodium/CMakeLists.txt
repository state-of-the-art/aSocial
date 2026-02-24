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

project(sodium LANGUAGES C)

include(${CMAKE_CURRENT_LIST_DIR}/../../common.cmake)

include(ProcessorCount)
ProcessorCount(CPU_N)

include(ExternalProject)
ExternalProject_Add(lib_${PROJECT_NAME}
  URL               ${lib_${PROJECT_NAME}_url}
  URL_HASH          ${lib_${PROJECT_NAME}_hash}
  DOWNLOAD_DIR      "${LIBS_DOWNLOAD_CACHE_DIR}/${PROJECT_NAME}"
  PREFIX            "${CMAKE_BINARY_DIR}/_build"
  INSTALL_DIR       "${LIBS_RESULT_DIR}"
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --disable-static --prefix=<INSTALL_DIR>
  BUILD_COMMAND     make -j${CPU_N}
  INSTALL_COMMAND   make -j${CPU_N} install
  BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libsodium.so
  LOG_OUTPUT_ON_FAILURE true
)
