# LevelDB dependency

set(LEVELDB_VERSION "1.22")
set(LEVELDB_HASH "8d310af5cfb53dc836bfb412ff4b3c8aea578627")

# Download the dependency
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/src/leveldb-${LEVELDB_VERSION}")
  message("Downloading LevelDB ${LEVELDB_VERSION} sources")
  set(leveldb_url "https://github.com/google/leveldb/archive/${LEVELDB_VERSION}.tar.gz")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/src")
  file(DOWNLOAD "${leveldb_url}" "${CMAKE_CURRENT_BINARY_DIR}/lib/src/leveldb.tar.gz" EXPECTED_HASH "SHA1=${LEVELDB_HASH}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_CURRENT_BINARY_DIR}/lib/src/leveldb.tar.gz"
                  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/src")
  if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/src/leveldb-${LEVELDB_VERSION}")
    message(FATAL_ERROR "Unable to download/unpack the LevelDB dependency")
  endif()
endif()

# Preparing config for dependency
set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "LevelDB option" FORCE)
set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "LevelDB option" FORCE)

add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/lib/src/leveldb-${LEVELDB_VERSION}" "${CMAKE_CURRENT_BINARY_DIR}/lib/leveldb")
set_target_properties(leveldb PROPERTIES COMPILE_FLAGS "-w") # Ignoring warnings from the dependency
set_property(TARGET leveldb PROPERTY POSITION_INDEPENDENT_CODE ON)

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:leveldb>)
endif()

# Link dependency to the project
target_link_libraries(${PROJECT_NAME} PRIVATE leveldb)
