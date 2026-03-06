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
project(dbkv-rocksdb LANGUAGES CXX VERSION 0.1.0)

# To build just the plugin
if(NOT DEFINED PLUGINS_SOURCE_DIR)
    set(PLUGINS_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()
include(${PLUGINS_SOURCE_DIR}/plugin.cmake)

# Additional library deps
add_dependencies(${PROJECT_NAME} lib_rocksdb)
add_dependencies(${PROJECT_NAME}_impl lib_rocksdb)

target_link_libraries(${PROJECT_NAME} PRIVATE -lrocksdb -lpthread -ldl Qt::Protobuf)
target_link_libraries(${PROJECT_NAME}_impl PRIVATE -lrocksdb -lpthread -ldl Qt::Protobuf)

target_link_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/lib)
target_link_directories(${PROJECT_NAME}_impl PRIVATE ${LIBS_RESULT_DIR}/lib)

target_include_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/include)
target_include_directories(${PROJECT_NAME}_impl PRIVATE ${LIBS_RESULT_DIR}/include)

# Last thing is to build tests if needed
include(${PLUGINS_SOURCE_DIR}/../tests.cmake)
