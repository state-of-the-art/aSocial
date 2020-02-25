cmake_minimum_required(VERSION 3.13)

project(asocial VERSION 0.1.0)

if(ANDROID)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(ANDROID_STL "c++_shared" CACHE STRING "Force to prevent issues" FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Options & user parameters
set(BUILD_MAIN_MODULE "frontend" CACHE STRING "The main module to build")
set(BUILD_EXCLUDE_MODULES "" CACHE STRING "List of modules to exclude, separated by ';'")

# System parameters
include(common.cmake)

# Add targets to check the source code format
set(CLANG_FORMAT_BIN_NAME clang-format-10)
set(CLANG_FORMAT_EXCLUDE_PATTERNS "build/" "/libs/" ${CMAKE_BINARY_DIR})
find_package(ClangFormat)

# Build common libs
include(libs/libs.cmake)

# Modules build
include(modules/modules.cmake)

# Make sure the source code is well formatted
add_dependencies(${PROJECT_NAME}-${BUILD_MAIN_MODULE} format-check)

# Packaging
add_custom_target(package ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Ok, package is done"
)

if(ANDROID)
  # TODO Making an addition to the deployment JSON file
  file(READ "${CMAKE_BINARY_DIR}/android_deployment_settings.json" deploy_data_in)
  string(REPLACE "\"ndk\":"
    "\"sdkBuildToolsRevision\": \"${ANDROID_SDK_BUILD_TOOLS}\",\n  \"ndk\":" deploy_data_in "${deploy_data_in}")
  file(WRITE "${CMAKE_BINARY_DIR}/android_deployment_settings.json" "${deploy_data_in}")
  add_custom_command(TARGET package PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_BINARY_DIR}/android-build/${PROJECT_NAME}-${BUILD_MAIN_MODULE}.apk"
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}_$<JOIN:${BUILD_ABIS},->.apk"
  )
  add_dependencies(package apk)
endif()

foreach(abi ${BUILD_ABIS})
  set(main_target_name "${PROJECT_NAME}-${BUILD_MAIN_MODULE}")
  if(NOT abi STREQUAL BUILD_ABI)
    set(main_target_name "${main_target_name}-${abi}-builder")
  endif()
  unset(libraries_to_pack_${abi})
  foreach(dir ${LIBS_LIB_DIRS_${abi}} ${MODULES_LIB_DIRS_${abi}})
    list(APPEND libraries_to_pack_${abi} ${dir}/*.so)
  endforeach()

  if(ANDROID)
    add_custom_target(prepare_package_${abi} ALL
      COMMAND ${CMAKE_COMMAND} -E copy ${libraries_to_pack_${abi}} android-build/libs/${abi}
      DEPENDS ${main_target_name}
    )

    # Strip binaries for release build
    if(CMAKE_BUILD_TYPE STREQUAL Release)
      add_custom_command(TARGET prepare_package_${abi} PRE_BUILD
        COMMAND ${CMAKE_BINARY_DIR}/strip_app_${abi}.exe -s android-build/libs/${abi}/*.so
      )
    endif()

    add_dependencies(apk prepare_package_${abi})
  else()
    # TODO: Check how that will work with a number of ABIS
    add_custom_target(prepare_package_${abi} ALL
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_NAME}-${PROJECT_VERSION}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_NAME}-${PROJECT_VERSION}/bin ${PROJECT_NAME}-${PROJECT_VERSION}/lib
      COMMAND ${CMAKE_COMMAND} -E copy ${libraries_to_pack_${abi}} ${PROJECT_NAME}-${PROJECT_VERSION}/lib
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${main_target_name}> ${PROJECT_NAME}-${PROJECT_VERSION}/bin/${PROJECT_NAME}
      DEPENDS ${main_target_name}
    )

    # Strip binaries for release build
    if(CMAKE_BUILD_TYPE STREQUAL Release)
      add_custom_command(TARGET prepare_package_${abi} PRE_BUILD
        COMMAND ${CMAKE_BINARY_DIR}/strip_app_${abi}.exe -s ${PROJECT_NAME}-${PROJECT_VERSION}/bin/* ${PROJECT_NAME}-${PROJECT_VERSION}/lib/*
      )
    endif()
    add_custom_target(package_${abi} ALL
      COMMAND ${CMAKE_COMMAND} -E tar czf ${PROJECT_NAME}-${PROJECT_VERSION}_${abi}.tar.gz ${PROJECT_NAME}-${PROJECT_VERSION}
      DEPENDS prepare_package_${abi}
      BYPRODUCTS ${PROJECT_NAME}-${PROJECT_VERSION}_${abi}.tar.gz
    )
    add_dependencies(package package_${abi})
  endif()
endforeach()
