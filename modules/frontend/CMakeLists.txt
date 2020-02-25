cmake_minimum_required(VERSION 3.13)

project(asocial-frontend LANGUAGES CXX VERSION 0.1.0)

include(../../common.cmake)

find_package(Qt5 COMPONENTS Core Quick Sql REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")

if(ANDROID)
  find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
  add_library(${PROJECT_NAME} SHARED ${Execution_CPP} ${Execution_H} "${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc")
else()
  add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H} "${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc")
endif()

# Dependencies build
include(android.cmake)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Quick Qt5::Sql $<$<BOOL:${ANDROID}>:Qt5::AndroidExtras> $<$<BOOL:${ANDROID}>:-llog> )
target_link_options(${PROJECT_NAME} PUBLIC -Wl,--no-undefined)

# Attach project modules
target_link_directories(${PROJECT_NAME} PUBLIC ${MODULES_LIB_DIRS_${BUILD_ABI}})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${MODULES_INCLUDE_DIRS}")
foreach(module ${MODULES_LIST})
  message("Linking asocial-${module}")
  target_link_libraries(${PROJECT_NAME} PRIVATE asocial-${module}_${BUILD_ABI})
endforeach()
