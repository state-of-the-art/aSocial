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

project(integration-tests LANGUAGES C)

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

find_package(Qt6 REQUIRED COMPONENTS Test)

# Getting Kotik sources to embed them
file(GLOB_RECURSE Kotik_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "kotik/*.cpp")
file(GLOB_RECURSE Kotik_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "kotik/*.h")

add_executable(cli_integration_tests "cli_integration_tests.cpp" ${Kotik_CPP} ${Kotik_H})
target_link_libraries(cli_integration_tests PRIVATE Qt::Test)
