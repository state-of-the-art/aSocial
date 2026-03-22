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
find_package(Protobuf REQUIRED)

# We have to generate the files separately, otherwise it will spit out tons of errors
# Unfortunately Qt6.10.1 doesn't have a generated descriptor, so creating it now
qt_add_protobuf(${PROJECT_NAME}
  PROTO_FILES "${Protobuf_INCLUDE_DIR}/google/protobuf/descriptor.proto"
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

target_link_libraries(${PROJECT_NAME} PUBLIC Qt::Protobuf Qt::ProtobufWellKnownTypes)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/proto)

# protoc-gen-coredb: one protoc invocation with all entity protos so the pool contains every file
set(COREDB_GEN_DIR "${CMAKE_BINARY_DIR}/proto")
set(COREDB_ENTITY_RELS "")
foreach(_p IN LISTS ASocial_PROTO)
  string(REGEX REPLACE "^proto/" "" _rel "${_p}")
  list(APPEND COREDB_ENTITY_RELS "${_rel}")
endforeach()
list(SORT COREDB_ENTITY_RELS)
list(GET COREDB_ENTITY_RELS 0 COREDB_ANCHOR_REL)
list(JOIN COREDB_ENTITY_RELS "," COREDB_ENTITY_FILES_OPT)
set(COREDB_PLUGIN_OPT "entity_files=${COREDB_ENTITY_FILES_OPT}")

get_filename_component(COREDB_ANCHOR_DIR "${COREDB_ANCHOR_REL}" DIRECTORY)
set(COREDB_GEN_OUTS
  "${COREDB_GEN_DIR}/${COREDB_ANCHOR_DIR}/coredb.gen.h"
  "${COREDB_GEN_DIR}/${COREDB_ANCHOR_DIR}/coredb.gen.cpp"
  "${COREDB_GEN_DIR}/${COREDB_ANCHOR_DIR}/common.coredb.gen.h"
  "${COREDB_GEN_DIR}/${COREDB_ANCHOR_DIR}/common.coredb.gen.cpp"
)
foreach(_rel IN LISTS COREDB_ENTITY_RELS)
  string(REPLACE ".proto" ".coredb.gen" _rel_rep "${_rel}")
  list(APPEND COREDB_GEN_OUTS "${COREDB_GEN_DIR}/${_rel_rep}.h")
  list(APPEND COREDB_GEN_OUTS "${COREDB_GEN_DIR}/${_rel_rep}.cpp")
endforeach()

set(COREDB_PROTO_INPUTS "")
foreach(_rel IN LISTS COREDB_ENTITY_RELS)
  list(APPEND COREDB_PROTO_INPUTS "${CMAKE_CURRENT_SOURCE_DIR}/proto/${_rel}")
endforeach()

add_custom_command(
  OUTPUT ${COREDB_GEN_OUTS}
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    --plugin=protoc-gen-coredb=$<TARGET_FILE:protoc-gen-coredb>
    "--coredb_out=${COREDB_GEN_DIR}"
    "--coredb_opt=${COREDB_PLUGIN_OPT}"
    -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
    ${COREDB_ENTITY_RELS}
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/proto"
  DEPENDS ${COREDB_PROTO_INPUTS} protoc-gen-coredb
  COMMENT "Running protoc-gen-coredb (combined entity_files)"
  VERBATIM
)

target_sources(${PROJECT_NAME} PRIVATE ${COREDB_GEN_OUTS})
set_source_files_properties(${COREDB_GEN_OUTS} PROPERTIES GENERATED TRUE)
