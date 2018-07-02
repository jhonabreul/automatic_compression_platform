set(SOURCES
  src/compression_test.cpp
)

set(HEADERS
  include/zlib_compressor_test.hpp
  include/snappy_compressor_test.hpp
  include/lzo_compressor_test.hpp
  include/bzip2_compressor_test.hpp
  include/lzma_compressor_test.hpp
  include/fpc_compressor_test.hpp
)

add_executable(compression_test ${SOURCES} ${HEADERS})
target_link_libraries(compression_test
                      test_utilities
                      gtest
                      gmock
                      utils
                      compression
                      snappy
                      lzo_library
                      socket
)

include_directories(include)