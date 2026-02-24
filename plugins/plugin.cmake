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

# Common plugin parameters

# Actually includes aSocial core common cmake
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

# Building implementation static library
file(GLOB_RECURSE Implementation_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Implementation_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h" "src/*.h")
list(REMOVE_ITEM Implementation_CPP src/plugin.cpp)
list(REMOVE_ITEM Implementation_H src/plugin.h)

add_library(${PROJECT_NAME}_impl STATIC ${Implementation_CPP} ${Implementation_H})
target_include_directories(${PROJECT_NAME}_impl PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include")
target_compile_definitions(${PROJECT_NAME}_impl PRIVATE PLUGIN_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME}_impl PRIVATE PLUGIN_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME}_impl PRIVATE Qt::Core)

# Build plugin
qt_add_plugin(${PROJECT_NAME} SHARED CLASS_NAME Plugin)

target_sources(${PROJECT_NAME} PRIVATE src/plugin.cpp src/plugin.h)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "../libasocial-plugin-")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_VERSION="${PROJECT_VERSION}")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include")
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_impl)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core ${PROJECT_NAME}_impl)
