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
project(ui-gui LANGUAGES CXX VERSION 0.1.0)

# To build just the plugin
if(NOT DEFINED PLUGINS_SOURCE_DIR)
    set(PLUGINS_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()
include(${PLUGINS_SOURCE_DIR}/plugin.cmake)

# Additional library deps
find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Qml)
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

target_link_libraries(${PROJECT_NAME}_impl PUBLIC Qt::Quick Qt::QuickControls2 Qt::Qml)

# Collect QML files for installation alongside the binary
file(GLOB_RECURSE QML_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "qml/*.qml")

# Embedding qml files into the plugin
qt_add_qml_module(${PROJECT_NAME}_impl
    URI "${PROJECT_NAME}"
    RESOURCE_PREFIX /asocial/plugin
    QML_FILES ${QML_FILES}
    NO_PLUGIN # We don't need a separated plugin here
)
# Is needed for qml module to include dependency
target_include_directories(${PROJECT_NAME}_impl PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" asocial_proto)

# Copy QML files to build directory for deployment dependencies looking by linuxdeploy
add_custom_target(${PROJECT_NAME}_qml ALL)
foreach(qml_file ${QML_FILES})
    get_filename_component(qml_dir ${qml_file} DIRECTORY)
    add_custom_command(TARGET ${PROJECT_NAME}_qml POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${qml_dir/${PROJECT_NAME}}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${qml_file}
            ${CMAKE_BINARY_DIR}/${qml_file}/${PROJECT_NAME}
    )
endforeach()

# Last thing is to build tests if needed
include(${PLUGINS_SOURCE_DIR}/../tests.cmake)
