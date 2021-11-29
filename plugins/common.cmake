# Common plugin parameters

# Actually includes aSocial core common cmake
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

# Build libs
include(${CMAKE_CURRENT_LIST_DIR}/libs.cmake)

qt_add_plugin(${PROJECT_NAME} SHARED CLASS_NAME Plugin)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h" "src/*.h")

target_sources(${PROJECT_NAME} PRIVATE ${Execution_CPP} ${Execution_H})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "../libasocial-plugin-")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_VERSION="${PROJECT_VERSION}")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include")
