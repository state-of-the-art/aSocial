cmake_minimum_required(VERSION 3.16)
project(dbsql-sqlcipher LANGUAGES CXX VERSION 0.1.0)

find_package(Qt6 COMPONENTS Core REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core -lqsqlitecipher)

add_dependencies(${PROJECT_NAME} lib_qsqlcipher)
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/plugins/sqldrivers)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/include)
