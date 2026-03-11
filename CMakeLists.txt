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

project(asocial VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common parameters
include(common.cmake)

# Build plugins
include(plugins.cmake)

option(BUILD_INTEGRATION_TESTS "Enable integration tests to be built" OFF)

if(BUILD_INTEGRATION_TESTS)
  add_subdirectory(tests)
endif()

find_package(Qt6 REQUIRED COMPONENTS Protobuf ProtobufWellKnownTypes LinguistTools)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")
file(GLOB_RECURSE Execution_TS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "i18n/*.ts")

qt_add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core asocial_proto asocial_log)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${Execution_TS})
