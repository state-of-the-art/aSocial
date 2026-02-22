cmake_minimum_required(VERSION 3.16)
project(dbsql-sqlcipher LANGUAGES CXX VERSION 0.1.0)

find_package(Qt6 COMPONENTS Core Sql REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

# We need to link both dep static libs here
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Sql -lqsqlitecipher -ltomcrypt)

add_dependencies(${PROJECT_NAME} lib_qsqlcipher)
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/lib)
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/plugins/sqldrivers)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/include)
