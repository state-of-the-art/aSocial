cmake_minimum_required(VERSION 3.16)
project(database-json LANGUAGES CXX VERSION 0.1.0)

find_package(Qt6 COMPONENTS Core REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core)
