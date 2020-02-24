cmake_minimum_required(VERSION 3.13)

# List of the dirs and add as subdirs
file(GLOB children RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
set(project_libs_list "")

foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${child})
    list(APPEND project_libs_list "${child}")
  endif()
endforeach()

# Cleaning the variables
unset(LIBS_DEPENDS)
foreach(abi ${BUILD_ABIS})
  unset(LIBS_LIB_DIRS_${abi})
  unset(LIBS_INCLUDE_DIRS_${abi})
endforeach()

include(ExternalProject)
foreach(lib ${project_libs_list})
  message("Configure lib: ${lib}")

  set(LIB_BASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs/${lib})
  include(${CMAKE_CURRENT_LIST_DIR}/${lib}/config.cmake)

  ExternalProject_Add(lib_${lib}_sources
    URL               ${lib_${lib}_url}
    URL_HASH          ${lib_${lib}_hash}
    DOWNLOAD_DIR      libs/_download
    SOURCE_DIR        ${LIB_BASE_BINARY_DIR}/src
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )

  foreach(abi ${BUILD_ABIS})
    if(ANDROID)
      set(android_lib_args
        -D ANDROID_ABI=${abi}
        -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
        -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
      )
    endif()

    set(LIB_RESULT_DIR ${LIB_BASE_BINARY_DIR}/${abi})

    ExternalProject_Add(lib_${lib}_${abi}
      DEPENDS lib_${lib}_sources
      PREFIX libs/_build
      SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${lib}
      INSTALL_DIR ${LIB_RESULT_DIR}
      UPDATE_COMMAND ""
      PATCH_COMMAND ""
      BUILD_ALWAYS YES
      DOWNLOAD_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_ARGS
        -D LIB_BASE_BINARY_DIR=${LIB_BASE_BINARY_DIR}
        -D LIB_RESULT_DIR=${LIB_RESULT_DIR}

        -D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
        -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
        -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
        -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
        -D CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
        ${android_lib_args}
    )

    list(APPEND LIBS_LIB_DIRS_${abi} "${LIB_RESULT_DIR}/lib")
    list(APPEND LIBS_INCLUDE_DIRS_${abi} "${LIB_RESULT_DIR}/include")
    list(APPEND LIBS_DEPENDS lib_${lib}_${abi})

    #add_library(${PROJECT_NAME}-${module}_${abi} SHARED IMPORTED)
    #set_target_properties(${PROJECT_NAME}-${module}_${abi}
    #    PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${module}/libs/${abi}/lib${PROJECT_NAME}-${module}_${abi}.so)
    #add_dependencies(${PROJECT_NAME}-${module}_${abi} module_${PROJECT_NAME}_${module})
  endforeach()
endforeach()
