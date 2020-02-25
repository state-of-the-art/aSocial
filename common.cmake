set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set universal BUILD_ABI & BUILD_ABIS
if(ANDROID)
  set(BUILD_ABI ${ANDROID_ABI})
  foreach(abi armeabi-v7a arm64-v8a x86 x86_64)
    if(abi STREQUAL ${BUILD_ABI})
      list(APPEND BUILD_ABIS ${abi})
    elseif(ANDROID_BUILD_ABI_${abi})
      list(APPEND BUILD_ABIS ${abi})
    endif()
  endforeach()
else()
  set(BUILD_ABI ${CMAKE_SYSTEM_PROCESSOR})
  set(BUILD_ABIS "${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Search for the current and parent binary dirs contains settings for module
set(_root_binary_dir ${CMAKE_BINARY_DIR})
while(NOT EXISTS "${_root_binary_dir}/module_settings.cmake")
  get_filename_component(_root_binary_dir_new "${_root_binary_dir}" DIRECTORY)
  if("${_root_binary_dir_new}" STREQUAL "${_root_binary_dir}")
    unset(_root_binary_dir)
    break()
  endif()
  set(_root_binary_dir "${_root_binary_dir_new}")
endwhile()

if(DEFINED _root_binary_dir)
  message("Include module settings: ${_root_binary_dir}/module_settings.cmake for ${CMAKE_CURRENT_SOURCE_DIR}")
  include(${_root_binary_dir}/module_settings.cmake)

  # Store the strip command to use it later for release stripping
  if(NOT EXISTS "${_root_binary_dir}/strip_app_${BUILD_ABI}.exe")
    if(CMAKE_HOST_UNIX)
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_STRIP}" "${_root_binary_dir}/strip_app_${BUILD_ABI}.exe")
    else()
      # TODO: Check how it's working
      file(COPY "${CMAKE_STRIP}" DESTINATION "${_root_binary_dir}")
      get_filename_component(_strip_name "${CMAKE_STRIP}" NAME)
      file(RENAME "${_root_binary_dir}/${_strip_name}" "${_root_binary_dir}/strip_app_${BUILD_ABI}.exe")
    endif()
  endif()
endif()

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
