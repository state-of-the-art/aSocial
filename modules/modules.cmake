cmake_minimum_required(VERSION 3.13)

# List of the dirs and add as subdirs
file(GLOB children RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
set(MODULES_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/modules)
set(MODULES_LIST "")

foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${child})
    if("${child}" STREQUAL "${BUILD_MAIN_MODULE}")
      # Skipping module if it's the main one
    elseif("${child}" IN_LIST "${BUILD_EXCLUDE_MODULES}")
      # Skip module if it's in the exclude list
    else()
      list(APPEND MODULES_LIST "${child}")
    endif()
  endif()
endforeach()

if(ANDROID)
  set(android_module_args
    -D ANDROID_ABI=${ANDROID_ABI}
    -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
    -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
  )
  foreach(abi ${BUILD_ABIS})
    list(APPEND android_module_args -D ANDROID_BUILD_ABI_${abi}=${ANDROID_BUILD_ABI_${abi}})
  endforeach()
endif()

include(ExternalProject)
foreach(module ${MODULES_LIST})
  message("Configure module: ${module}")

  if(ANDROID)
    set(_install_command ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/android-build/libs <INSTALL_DIR>/lib)
  else()
    set(_install_command ${CMAKE_COMMAND} -E copy
      <BINARY_DIR>/lib${PROJECT_NAME}-${module}.so
      <INSTALL_DIR>/lib/${BUILD_ABI}/lib${PROJECT_NAME}-${module}_${BUILD_ABI}.so
    )
  endif()

  unset(_build_byproducts)
  foreach(abi ${BUILD_ABIS})
    list(APPEND _build_byproducts <INSTALL_DIR>/lib/${abi}/lib${PROJECT_NAME}-${module}_${abi}.so)
  endforeach()

  # There is no point of transfering any additional params -
  # they will be eaten by Qt5AndroidSupport.cmake during
  # preparation of the additional ABI builds
  ExternalProject_Add(module_${module}
    DEPENDS ${LIBS_DEPENDS}
    PREFIX ${MODULES_BIN_DIR}/_build
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/${module}"
    INSTALL_DIR ${MODULES_BIN_DIR}/${module}
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_ALWAYS YES
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ${_install_command}
    CMAKE_ARGS
      ${android_module_args}
      -D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
      -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
      -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
      -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
      -D CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
    BUILD_BYPRODUCTS ${_build_byproducts}
  )

  foreach(abi ${BUILD_ABIS})
    set(module_target_name "${PROJECT_NAME}-${module}_${abi}")
    add_library(${module_target_name} SHARED IMPORTED)
    set_target_properties(${module_target_name}
      PROPERTIES IMPORTED_LOCATION ${MODULES_BIN_DIR}/${module}/lib/${abi}/lib${module_target_name}.so)
    add_dependencies(${module_target_name} module_${module})
  endforeach()
endforeach()

# Add modules include directories
foreach(mod ${MODULES_LIST})
  list(APPEND modules_includes "${CMAKE_CURRENT_LIST_DIR}/${mod}/include")
  foreach(abi ${BUILD_ABIS})
    list(APPEND MODULES_LIB_DIRS_${abi} "${MODULES_BIN_DIR}/${mod}/lib/${abi}")
  endforeach()
endforeach()

if(NOT EXISTS ${CMAKE_BINARY_DIR}/module_settings.cmake)
  # Prepare config file for the modules
  string(REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  file(WRITE ${CMAKE_BINARY_DIR}/module_settings.cmake "\
# Modules config file
set(PROJECT_VERSION ${PROJECT_VERSION} CACHE STRING \"The entire project version\" FORCE)
set(LIBS_LIB_DIRS ${LIBS_LIB_DIRS} CACHE STRING \"Common libs lib dirs\" FORCE)
set(MODULES_LIST ${MODULES_LIST} CACHE STRING \"Modules list\" FORCE)
set(MODULES_INCLUDE_DIRS ${modules_includes} CACHE STRING \"Modules include dirs list\" FORCE)
set(ANDROID_ALLOW_UNDEFINED_SYMBOLS OFF CACHE BOOL \"Don't use no-undefined for modules\" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS \"${CMAKE_SHARED_LINKER_FLAGS}\" CACHE STRING \"Linker flags\" FORCE)"
  )

  foreach(abi ${BUILD_ABIS})
    file(APPEND "${CMAKE_BINARY_DIR}/module_settings.cmake"
      "\nset(LIBS_LIB_DIRS_${abi} ${LIBS_LIB_DIRS_${abi}} CACHE STRING \"Common libs lib dirs for ${abi}\" FORCE)"
      "\nset(MODULES_LIB_DIRS_${abi} ${MODULES_LIB_DIRS_${abi}} CACHE STRING \"Modules lib dirs for ${abi}\" FORCE)"
      "\nset(LIBS_INCLUDE_DIRS_${abi} ${LIBS_INCLUDE_DIRS_${abi}} CACHE STRING \"Common libs include dirs for ${abi}\" FORCE)"
    )
  endforeach()
endif()

message("Configure main module ${BUILD_MAIN_MODULE}")
set(ANDROID_PACKAGE_SOURCE_DIR "${MODULES_BIN_DIR}/${BUILD_MAIN_MODULE}/android" CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${BUILD_MAIN_MODULE})

# Add deps on the modules for the main module
foreach(module ${MODULES_LIST})
  foreach(abi ${BUILD_ABIS})
    set(main_target_name "${PROJECT_NAME}-${BUILD_MAIN_MODULE}")
    if(NOT abi STREQUAL BUILD_ABI)
      set(main_target_name "${main_target_name}-${abi}-builder")
    endif()

    add_dependencies(${main_target_name} ${PROJECT_NAME}-${module}_${abi})
  endforeach()
endforeach()
