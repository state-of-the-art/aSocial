file(GLOB_RECURSE SimpleQtCryptor_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "libs/simpleqtcryptor/*.cpp")
file(GLOB_RECURSE SimpleQtCryptor_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "libs/simpleqtcryptor/*.h")

list(APPEND Library_CPP ${SimpleQtCryptor_CPP})
list(APPEND Library_H ${SimpleQtCryptor_H})
