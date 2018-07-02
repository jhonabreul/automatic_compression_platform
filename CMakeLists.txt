cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(AutoComp VERSION 0.1 LANGUAGES CXX C)

find_package(Threads)

include(ExternalProject)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Google Test ##################################################################

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(external/googletest)

################################################################################

# zlib #########################################################################

ExternalProject_Add(zlib
  PREFIX              ${CMAKE_BINARY_DIR}/external
  URL                 file://${CMAKE_SOURCE_DIR}/external/zlib-1.2.11.tar.gz
  URL_MD5             1c9f62f0778697a09d36121ead88e08e
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
  CONFIGURE_COMMAND   CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
    ${CMAKE_BINARY_DIR}/external/src/zlib/configure
    --prefix=${CMAKE_BINARY_DIR}
    --enable-shared
  INSTALL_COMMAND make install
)

add_library(zlib_library STATIC IMPORTED)
add_dependencies(zlib_library zlib)
set_target_properties(zlib_library
                      PROPERTIES
                        IMPORTED_LOCATION
                          ${CMAKE_BINARY_DIR}/lib/libz.so
)


################################################################################

# Snappy #######################################################################

SET(SNAPPY_BUILD_TESTS OFF CACHE BOOL "Build snappy unit tests")
SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build snappy shared library")
add_subdirectory(external/snappy)

################################################################################

# lzo ##########################################################################

ExternalProject_Add(lzo
  PREFIX              ${CMAKE_BINARY_DIR}/external
  URL                 file://${CMAKE_SOURCE_DIR}/external/lzo-2.10.tar.gz
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
  CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  CONFIGURE_COMMAND   CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
    ${CMAKE_BINARY_DIR}/external/src/lzo/configure
    --prefix=${CMAKE_BINARY_DIR}
    --enable-shared
  INSTALL_COMMAND make install
)

add_library(lzo_library STATIC IMPORTED)
add_dependencies(lzo_library lzo)
set_target_properties(lzo_library
                      PROPERTIES
                        IMPORTED_LOCATION
                          ${CMAKE_BINARY_DIR}/lib/liblzo2.so
)


################################################################################

# bzip2 ########################################################################

ExternalProject_Add(bzip2
  PREFIX              ${CMAKE_BINARY_DIR}/external
  URL                 file://${CMAKE_SOURCE_DIR}/external/bzip2-1.0.6.tar.gz
  URL_MD5             00b516f4704d4a7cb50a1d97e6e8e15b
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
  CONFIGURE_COMMAND   ""
  BUILD_IN_SOURCE TRUE
  BUILD_COMMAND make -f Makefile-libbz2_so && make
  INSTALL_COMMAND make install PREFIX=${CMAKE_BINARY_DIR}
)

add_library(bzip2_library STATIC IMPORTED)
add_dependencies(bzip2_library bzip2)
set_target_properties(bzip2_library
                      PROPERTIES
                        IMPORTED_LOCATION
                          ${CMAKE_BINARY_DIR}/lib/libbz2.a
)

################################################################################

# lzma #########################################################################

ExternalProject_Add(lzma
  PREFIX              ${CMAKE_BINARY_DIR}/external
  URL                 file://${CMAKE_SOURCE_DIR}/external/xz-5.2.4.tar.gz
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
  CONFIGURE_COMMAND   CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
    ${CMAKE_BINARY_DIR}/external/src/lzma/configure
    --prefix=${CMAKE_BINARY_DIR}
    --enable-shared
    --disable-xz
    --disable-xzdec
    --disable-lzmadec
    --disable-lzmainfo
    --disable-doc
  INSTALL_COMMAND make install
)

add_library(lzma_library STATIC IMPORTED)
add_dependencies(lzma_library lzma)
set_target_properties(lzma_library
                      PROPERTIES
                        IMPORTED_LOCATION
                          ${CMAKE_BINARY_DIR}/lib/liblzma.so
)

################################################################################

# g3log ########################################################################

SET(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/external" CACHE PATH
    "g3log install prefix")
SET(CMAKE_BUILD_TYPE Release CACHE STRING "g3log release type")
SET(ADD_FATAL_EXAMPLE OFF CACHE BOOL "")
SET(USE_DYNAMIC_LOGGING_LEVELS ON)
add_subdirectory(external/g3log)

################################################################################

# protobuf #####################################################################

ExternalProject_Add(protobuf
  PREFIX              ${CMAKE_BINARY_DIR}/external
  URL                 file://${CMAKE_SOURCE_DIR}/external/protobuf-cpp-3.6.0.tar.gz
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
  CONFIGURE_COMMAND   CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
    ${CMAKE_BINARY_DIR}/external/src/protobuf/configure
    --prefix=${CMAKE_BINARY_DIR}
  INSTALL_COMMAND make install
)

add_library(protobuf_library STATIC IMPORTED)
add_dependencies(protobuf_library protobuf)
set_target_properties(protobuf_library
                        PROPERTIES
                          IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libprotobuf.so)

add_library(protobuf_lite_library STATIC IMPORTED)
add_dependencies(protobuf_lite_library protobuf)
set_target_properties(protobuf_lite_library
                        PROPERTIES
                          IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libprotobuf-lite.so)

add_library(protoc_library STATIC IMPORTED)
add_dependencies(protoc_library protobuf)
set_target_properties(protoc_library
                        PROPERTIES
                          IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libprotoc.so)


################################################################################

# libconfig ####################################################################

ExternalProject_Add(libconfig
  PREFIX              ${CMAKE_BINARY_DIR}/external
  URL                 file://${CMAKE_SOURCE_DIR}/external/libconfig-1.7.2.tar.gz
  URL_MD5             6bd98ee3a6e6b9126c82c916d7a9e690
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
  CONFIGURE_COMMAND   CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
    ${CMAKE_BINARY_DIR}/external/src/libconfig/configure
    --prefix=${CMAKE_BINARY_DIR}
    --enable-shared
    --disable-examples
  BUILD_COMMAND cp ${CMAKE_BINARY_DIR}/external/src/libconfig/lib/libconfig.h
    ${CMAKE_BINARY_DIR}/external/src/libconfig-build/lib/ && make
  INSTALL_COMMAND make install
)

add_library(libconfig_library STATIC IMPORTED)
add_dependencies(libconfig_library libconfig)
set_target_properties(libconfig_library
                      PROPERTIES
                        IMPORTED_LOCATION
                          ${CMAKE_BINARY_DIR}/lib/libconfig.so
)

add_library(libconfig++_library STATIC IMPORTED)
add_dependencies(libconfig++_library libconfig++)
set_target_properties(libconfig++_library
                      PROPERTIES
                        IMPORTED_LOCATION
                          ${CMAKE_BINARY_DIR}/lib/libconfig++.so
)

################################################################################

if(BW_TEST)
  add_definitions(-DBANDWIDTH_TEST)
endif(BW_TEST)

if(MEASURE_TIMES)
  add_definitions(-DMEASURE_COMPRESSION_TIME)
endif(MEASURE_TIMES)

include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/external/googletest/googletest/include")
include_directories("${CMAKE_SOURCE_DIR}/external/googletest/googlemock/include")
include_directories("${CMAKE_SOURCE_DIR}/external/snappy")
include_directories("${PROJECT_BINARY_DIR}/external/snappy")
include_directories("${CMAKE_SOURCE_DIR}/external/g3log/src")

add_subdirectory(external/fpc)
add_subdirectory(src)
add_subdirectory(test)

if(DUMMY)
  add_subdirectory(dummy)
endif(DUMMY)
