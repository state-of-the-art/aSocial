# QSqlCipher dependency

set(SQLCIPHER_VERSION "3.4.2")
set(SQLCIPHER_HASH "c06efc6b79dc0c5782e3cb7ca31158587927d21a")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/qsqlcipher" "${CMAKE_CURRENT_BINARY_DIR}/libs/qsqlcipher")

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:qsqlcipher>)
endif()

# Link dependency to the project
target_link_libraries(${PROJECT_NAME} PRIVATE qsqlcipher)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/qsqlcipher")
