cmake_minimum_required(VERSION 3.16)
project(dbsql-sqlite LANGUAGES CXX VERSION 0.1.0)

find_package(Qt6 COMPONENTS Core Sql REQUIRED)

include(${PLUGINS_SOURCE_DIR}/common.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Sql)

target_include_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/include)
