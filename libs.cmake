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

if(NOT DEFINED ASOCIAL_LIBS_DIR)
    set(ASOCIAL_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
endif()
#message("Processing libs in ${ASOCIAL_LIBS_DIR}")

# List of the dirs and add as subdirs
set(_libs_dir "${ASOCIAL_LIBS_DIR}")
unset(ASOCIAL_LIBS_DIR)
file(GLOB childs RELATIVE "${_libs_dir}" "${_libs_dir}/*")
unset(LIBS_LIST)
foreach(child ${childs})
    if(IS_DIRECTORY ${_libs_dir}/${child})
        list(APPEND LIBS_LIST "${child}")
    endif()
endforeach()

foreach(lib ${LIBS_LIST})
    if(DEFINED lib_${lib}_loaded)
        #message("Skipping already loaded lib: ${lib}")
        continue()
    endif()
    set(lib_${lib}_loaded ON)
    message("Configure lib: ${lib}")

    include(${_libs_dir}/${lib}/config.cmake)

    add_subdirectory("${_libs_dir}/${lib}" "${CMAKE_BINARY_DIR}/libs_build/${lib}")
endforeach()
