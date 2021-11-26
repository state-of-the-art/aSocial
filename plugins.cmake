cmake_minimum_required(VERSION 3.16)

# List of the dirs and add as subdirs
set(_plugins_dir "${CMAKE_CURRENT_LIST_DIR}/plugins")
file(GLOB childs RELATIVE ${_plugins_dir} "${_plugins_dir}/*")
set(PLUGINS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins)
set(PLUGINS_LIST "")

foreach(child ${childs})
    if(EXISTS "${_plugins_dir}/${child}/CMakeLists.txt")
        list(APPEND PLUGINS_LIST "${child}")
    endif()
endforeach()

foreach(plugin ${PLUGINS_LIST})
    message("Configure plugin: ${plugin}")
    add_subdirectory("${_plugins_dir}/${plugin}" "${PLUGINS_BIN_DIR}")
endforeach()
