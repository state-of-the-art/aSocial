cmake_minimum_required(VERSION 3.16)
project(comm-tox LANGUAGES CXX VERSION 0.1.0)

find_package(Qt6 COMPONENTS Core REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core -ltoxcore)

add_dependencies(${PROJECT_NAME} lib_toxcore)
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/lib)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBS_RESULT_DIR}/include)
