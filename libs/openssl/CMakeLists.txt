cmake_minimum_required(VERSION 3.13)

project(openssl LANGUAGES C)

include(../../common.cmake)

set(src_dir ${LIB_BASE_BINARY_DIR}/src)
set(output_dir ${LIB_RESULT_DIR})

include(ProcessorCount)
ProcessorCount(CPU_N)

if(ANDROID)
  if(BUILD_ABI STREQUAL "armeabi-v7a")
    set(openssl_arch android-arm -march=armv7-a -D_FILE_OFFSET_BITS=32 -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL})
  elseif(BUILD_ABI STREQUAL "arm64-v8a")
    set(openssl_arch android-arm64 -D_FILE_OFFSET_BITS=64 -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL})
  elseif(BUILD_ABI STREQUAL "x86")
    set(openssl_arch android-x86 -D_FILE_OFFSET_BITS=32 -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL})
  elseif(BUILD_ABI STREQUAL "x86_64")
    set(openssl_arch android64-x86_64 -D_FILE_OFFSET_BITS=64 -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL})
  else()
    message(FATAL_ERROR "Unknown ABI to build '${BUILD_ABI}'")
  endif()
  set(openssl_env export ANDROID_NDK_HOME=${ANDROID_NDK} PATH=${CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN}/bin:$ENV{PATH} CC=clang &&)
else()
  set(openssl_arch linux-${BUILD_ABI})
endif()

set(openssl_opts -fPIC -fstack-protector-all no-idea no-camellia no-seed no-bf no-cast no-rc2 no-rc4
    no-rc5 no-md2 no-md4 no-sock no-ssl3 no-dsa no-dh no-rfc3779
    no-whirlpool no-srp no-mdc2 no-engine no-srtp no-gost)

include(ExternalProject)
ExternalProject_Add(ex_openssl
  PREFIX            ${CMAKE_BINARY_DIR}/_build
  #DOWNLOAD_DIR      ${LIB_BASE_BINARY_DIR}/_download
  SOURCE_DIR        ${src_dir}
  INSTALL_DIR       ${output_dir}
  CONFIGURE_COMMAND ${openssl_env} <SOURCE_DIR>/Configure shared ${openssl_arch} ${openssl_opts} --prefix=<INSTALL_DIR>
  BUILD_COMMAND     ${openssl_env} make -j${CPU_N} build_libs
  INSTALL_COMMAND   make -j${CPU_N} install_dev
)
