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

# List of the dirs and add as subdirs
set(_plugins_dir "${CMAKE_CURRENT_LIST_DIR}/plugins")
file(GLOB childs RELATIVE ${_plugins_dir} "${_plugins_dir}/*")
set(PLUGINS_SOURCE_DIR ${asocial_SOURCE_DIR}/plugins)
set(PLUGINS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins)
set(PLUGINS_LIST "")

foreach(child ${childs})
  if(EXISTS "${_plugins_dir}/${child}/CMakeLists.txt")
    # Filter the gui plugins if we building minimal dist
    if("${child}" MATCHES "^ui-gui")
      if(ASOCIAL_CLI_ONLY)
        message("Skipping plugin due to CLI only build: ${child}")
        continue()
      endif()
    endif()
    list(APPEND PLUGINS_LIST "${child}")
  endif()
endforeach()

foreach(plugin ${PLUGINS_LIST})
  message("Configure plugin: ${plugin}")
  add_subdirectory("${_plugins_dir}/${plugin}" "${PLUGINS_BIN_DIR}/${plugin}")
endforeach()
