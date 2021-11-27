cmake_minimum_required(VERSION 3.16)

message("Processing libs in ${CMAKE_CURRENT_SOURCE_DIR}/libs")

# List of the dirs and add as subdirs
set(_libs_dir "${CMAKE_CURRENT_SOURCE_DIR}/libs")
file(GLOB childs RELATIVE "${_libs_dir}" "${_libs_dir}/*")
unset(LIBS_LIST)
foreach(child ${childs})
    if(IS_DIRECTORY ${_libs_dir}/${child})
        list(APPEND LIBS_LIST "${child}")
    endif()
endforeach()

include(ExternalProject)
foreach(lib ${LIBS_LIST})
    message("Configure lib: ${lib}")

    set(LIB_BASE_BINARY_DIR ${CMAKE_BINARY_DIR}/libs/${lib})

    include(${_libs_dir}/${lib}/config.cmake)

    #include(FetchContent)
    #FetchContent_Declare(lib_${lib}
    #  URL      ${lib_${lib}_url}
    #  URL_HASH ${lib_${lib}_hash}
    #)
    #FetchContent_Populate(lib_${lib}_sources)
    #FetchContent_MakeAvailable()
    #ExternalProject_Add(lib_${lib}_sources
    #  URL               ${lib_${lib}_url}
    #  URL_HASH          ${lib_${lib}_hash}
    #  DOWNLOAD_DIR      libs/_download
    #  SOURCE_DIR        "${LIBS_SRC_DIR}/${lib}"
    #  UPDATE_COMMAND ""
    #  PATCH_COMMAND ""
    #  CONFIGURE_COMMAND ""
    #  BUILD_COMMAND ""
    #  INSTALL_COMMAND ""
    #)

    add_subdirectory("${_libs_dir}/${lib}")
endforeach()
