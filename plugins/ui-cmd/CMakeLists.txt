cmake_minimum_required(VERSION 3.16)
project(ui-cmd LANGUAGES CXX VERSION 0.1.0)

find_package(Qt6 COMPONENTS Core REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core -llib_qr_code_generator)

add_dependencies(${PROJECT_NAME} lib_toxcore lib_qr_code_generator)
target_link_directories(${PROJECT_NAME} PRIVATE ${lib_qr_code_generator_BINARY_DIR}/lib ${qr_code_generator_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${lib_cli_INCLUDE_DIR} ${lib_qr_code_generator_INCLUDE_DIR})
