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

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Variable useful for download cache
if(NOT DEFINED LIBS_DOWNLOAD_CACHE_DIR)
    set(LIBS_DOWNLOAD_CACHE_DIR ${CMAKE_BINARY_DIR}/../downloads)
endif()
set(LIBS_RESULT_DIR ${CMAKE_BINARY_DIR}/libs)

# Main Qt core component
find_package(Qt6 COMPONENTS Core REQUIRED)

# Build common libs
set(ASOCIAL_LIBS_DIR ${CMAKE_CURRENT_LIST_DIR}/libs)
include(${CMAKE_CURRENT_LIST_DIR}/libs.cmake)
unset(ASOCIAL_LIBS_DIR)

# Build project libs
include(${CMAKE_CURRENT_LIST_DIR}/libs.cmake)
