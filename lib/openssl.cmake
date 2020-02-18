set(EX_OPENSSL_LIB_DIR ${CMAKE_BINARY_DIR}/openssl/install/lib)
set(EX_OPENSSL_INCLUDE_DIR ${CMAKE_BINARY_DIR}/openssl/install/include)

if(NOT TARGET ex_openssl)
  include(ProcessorCount)
  ProcessorCount(CPU_N)

  set(openssl_arch linux-x86_64)
  # TODO: set(openssl_arch "android-<arch> -D__ANDROID_API__=21")
  # android-arm -march=armv7-a -D_FILE_OFFSET_BITS=32
  # android-x86 -D_FILE_OFFSET_BITS=32
  # android64-x86_64 -D_FILE_OFFSET_BITS=64
  # android-arm64 -D_FILE_OFFSET_BITS=64
  set(openssl_opts -fPIC -fstack-protector-all no-idea no-camellia no-seed no-bf no-cast no-rc2 no-rc4
      no-rc5 no-md2 no-md4 no-sock no-ssl3 no-dsa no-dh no-rfc3779
      no-whirlpool no-srp no-mdc2 no-engine no-srtp no-gost)

  include(ExternalProject)
  ExternalProject_Add(ex_openssl
    PREFIX            ${CMAKE_BINARY_DIR}/openssl
    URL               https://www.openssl.org/source/openssl-1.1.1d.tar.gz
    URL_HASH          SHA1=056057782325134b76d1931c48f2c7e6595d7ef4
    CONFIGURE_COMMAND <SOURCE_DIR>/Configure shared ${openssl_arch} ${openssl_opts} --prefix=${CMAKE_BINARY_DIR}/openssl/install
    BUILD_COMMAND     make -j${CPU_N} build_libs
    INSTALL_COMMAND   make -j${CPU_N} install_dev
    BUILD_BYPRODUCTS  ${EX_OPENSSL_LIB_DIR}/libcrypto.so ${EX_OPENSSL_LIB_DIR}/libssl.so
  )
endif()

add_library(ExOpenSSL::Crypto SHARED IMPORTED)
set_target_properties(ExOpenSSL::Crypto PROPERTIES IMPORTED_LOCATION ${EX_OPENSSL_LIB_DIR}/libcrypto.so)
add_dependencies(ExOpenSSL::Crypto ex_openssl)

add_library(ExOpenSSL::SSL SHARED IMPORTED)
set_target_properties(ExOpenSSL::SSL PROPERTIES IMPORTED_LOCATION ${EX_OPENSSL_LIB_DIR}/libssl.so)
add_dependencies(ExOpenSSL::SSL ex_openssl)
