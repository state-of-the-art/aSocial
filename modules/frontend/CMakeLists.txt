cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

if(ANDROID)
  find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
  add_library(${PROJECT_NAME} SHARED ${Execution_CPP} ${Execution_H} "${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc")
  set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_BINARY_DIR}/android" CACHE INTERNAL "")
else()
  add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H} "${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc")
endif()

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:asocial>)
endif()

# Dependencies build
include(android.cmake)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Quick)

# Attach project modules
foreach(module ${project_modules_list})
  message("Linking ${PROJECT_NAME}-${module}")
  target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-${module})
endforeach()

if(ANDROID)
  list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
  string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
  set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")
  message("Additional android libs: ${ANDROID_EXTRA_LIBS}")

  target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::AndroidExtras)

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND sh "${CMAKE_SOURCE_DIR}/tools/build-apk.sh" $<TARGET_FILE:asocial> ${ANDROID_EXTRA_LIBS} ${ANDROID_PACKAGE_SOURCE_DIR})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
