# LevelDB dependency

set(LEVELDB_VERSION "1.22")
set(LEVELDB_HASH "8d310af5cfb53dc836bfb412ff4b3c8aea578627")

if(ANDROID)
  set(android_module_args
    -D ANDROID_ABI=${ANDROID_ABI}
    -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
    -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
  )
endif()

include(ExternalProject)
ExternalProject_Add(ex_leveldb
  PREFIX            ${CMAKE_CURRENT_BINARY_DIR}/libs/_build
  URL               https://github.com/google/leveldb/archive/${LEVELDB_VERSION}.tar.gz
  URL_HASH          SHA1=${LEVELDB_HASH}
  INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/libs/leveldb
  CMAKE_ARGS        -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR> # set the install prefix per generator
                    -D BUILD_SHARED_LIBS=OFF # build static libs
                    -D LEVELDB_BUILD_TESTS=OFF
                    -D LEVELDB_BUILD_BENCHMARKS=OFF
                    -D CMAKE_POSITION_INDEPENDENT_CODE=ON

                    -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                    -D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
                    -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
                    -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
                    -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
                    -D CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
                    ${android_module_args}
  BUILD_COMMAND     ${CMAKE_COMMAND} --build .
  BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libleveldb.a
)

ExternalProject_Get_Property(ex_leveldb INSTALL_DIR)
target_include_directories(${PROJECT_NAME} PRIVATE ${INSTALL_DIR}/include)

add_library(ExLevelDB STATIC IMPORTED)
set_target_properties(ExLevelDB PROPERTIES IMPORTED_LOCATION ${INSTALL_DIR}/lib/libleveldb.a)
add_dependencies(ExLevelDB ex_leveldb)

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:ExLevelDB>)
endif()

# Link dependency to the project
target_link_libraries(${PROJECT_NAME} PRIVATE ExLevelDB)
