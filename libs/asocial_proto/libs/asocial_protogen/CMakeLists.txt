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

project(asocial_protogen)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../common.cmake)

find_package(Protobuf REQUIRED)

add_executable(protoc-gen-coredb src/coredb.cpp)
target_link_libraries(protoc-gen-coredb PRIVATE protobuf::libprotobuf protobuf::libprotoc)

set(EXECUTABLE_PATH_PROTOC_GEN_COREDB "${CMAKE_CURRENT_BINARY_DIR}/protoc-gen-coredb")
