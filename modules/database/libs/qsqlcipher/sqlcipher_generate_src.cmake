cmake_minimum_required(VERSION 3.5)

set(SQLCIPHER_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(SQLCIPHER_INCLUDE_DIR "${SQLCIPHER_GEN_DIR}/include")

set(libs_include_dirs ${LIBS_INCLUDE_DIRS_${BUILD_ABI}})
list(TRANSFORM libs_include_dirs PREPEND -I) # TODO: potential error when >1 lib in the list
set(sqlcipher_opts --enable-shared=no --enable-tempstore=yes --with-crypto-lib=none --disable-tcl
  CFLAGS=-DSQLITE_HAS_CODEC\ -DSQLCIPHER_CRYPTO_OPENSSL\ -fPIC\ ${libs_include_dirs}
)

include(ExternalProject)
ExternalProject_Add(sqlcipher
  PREFIX            ${CMAKE_CURRENT_BINARY_DIR}
  URL               https://github.com/sqlcipher/sqlcipher/archive/v${SQLCIPHER_VERSION}.tar.gz
  URL_HASH          SHA1=${SQLCIPHER_HASH}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure ${sqlcipher_opts}
  BUILD_COMMAND     make sqlite3.c
  INSTALL_DIR       ${SQLCIPHER_INCLUDE_DIR}/sqlcipher
  INSTALL_COMMAND   ${CMAKE_COMMAND} -E copy sqlite3.c ${SQLCIPHER_GEN_DIR}
          COMMAND   ${CMAKE_COMMAND} -E copy sqlite3.h sqlite3ext.h <INSTALL_DIR>
  BUILD_BYPRODUCTS  <INSTALL_DIR>/sqlite3.h <INSTALL_DIR>/sqlite3ext.h ${SQLCIPHER_GEN_DIR}/sqlite3.c
)

set_property(
  SOURCE ${SQLCIPHER_INCLUDE_DIR}/sqlcipher/sqlite3.h
         ${SQLCIPHER_INCLUDE_DIR}/sqlcipher/sqlite3ext.h
         ${SQLCIPHER_GEN_DIR}/sqlite3.c
  PROPERTY SKIP_AUTOGEN ON
)
