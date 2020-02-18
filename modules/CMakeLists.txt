# List of the dirs and add as subdirs
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
set(project_modules_list "")

foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
    if("${child}" STREQUAL "${BUILD_MAIN_MODULE}")
    elseif("${child}" IN_LIST "${BUILD_EXCLUDE_MODULES}")
    else()
      list(APPEND project_modules_list "${child}")
    endif()
  endif()
endforeach()

foreach(module ${project_modules_list})
  message("Configure module: ${module}")
  add_subdirectory(${module})

  # Add other modules include directories
  foreach(mod ${project_modules_list})
    target_include_directories(${PROJECT_NAME}-${module} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${mod}/include")
  endforeach()
endforeach()

message("Configure main module ${BUILD_MAIN_MODULE}")
add_subdirectory(${BUILD_MAIN_MODULE})

foreach(module ${project_modules_list})
  target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${module}/include")
endforeach()
