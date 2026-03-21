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

project(asocial_proto LANGUAGES C)

include(${CMAKE_CURRENT_LIST_DIR}/../../common.cmake)

find_package(Qt6 REQUIRED COMPONENTS Protobuf ProtobufWellKnownTypes)

# We have to generate the files separately, otherwise it will spit out tons of errors
file(GLOB_RECURSE Google_PROTO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "proto/google/*.proto")
qt_add_protobuf(${PROJECT_NAME}
  PROTO_FILES ${Google_PROTO}
  PROTO_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/proto"
  GENERATE_PACKAGE_SUBFOLDERS
  OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/proto"
)

# Generating options separately, otherwise again issues with imports all over again
file(GLOB_RECURSE ASocialOptions_PROTO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "proto/asocial/*.proto")
list(FILTER ASocialOptions_PROTO INCLUDE REGEX "options_")

qt_add_protobuf(${PROJECT_NAME}
  PROTO_FILES ${ASocialOptions_PROTO}
  COPY_COMMENTS
  PROTO_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/proto"
  GENERATE_PACKAGE_SUBFOLDERS
  OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/proto"
)

# Finally generate the actual messages for Qt except for options
file(GLOB_RECURSE ASocial_PROTO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "proto/asocial/*.proto")
list(FILTER ASocial_PROTO EXCLUDE REGEX "options_")

qt_add_protobuf(${PROJECT_NAME}
  PROTO_FILES ${ASocial_PROTO}
  COPY_COMMENTS
  PROTO_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/proto"
  GENERATE_PACKAGE_SUBFOLDERS
  OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/proto"
)

target_link_libraries(${PROJECT_NAME} PUBLIC Qt::Protobuf Qt::ProtobufWellKnownTypes )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/proto ${PROJECT_NAME}_coredb)

# Custom generator for typical coredb operations
protobuf_generate(
    TARGET ${PROJECT_NAME}
    LANGUAGE coredb
    DEPENDENCIES protoc-gen-coredb
    PROTOS ${ASocial_PROTO}
    PLUGIN "protoc-gen-coredb=$<TARGET_FILE:protoc-gen-coredb>" # "$<TARGET_FILE:protoc-gen-coredb>" # "${CMAKE_BINARY_DIR}/libs_build/asocial_protogen/protoc-gen-coredb" #"${EXECUTABLE_PATH_PROTOC_GEN_COREDB}"
    #PROTOC_OPTIONS "--coredb_out=${CMAKE_CURRENT_BINARY_DIR}"
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
    IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
    GENERATE_EXTENSIONS .proto.gen.h
)
