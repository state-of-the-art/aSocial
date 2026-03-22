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

project(asocial_tpl LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../../common.cmake)

file(GLOB_RECURSE Lib_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Lib_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h" "src/*.h")

add_library(${PROJECT_NAME} SHARED ${Lib_CPP} ${Lib_H})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
