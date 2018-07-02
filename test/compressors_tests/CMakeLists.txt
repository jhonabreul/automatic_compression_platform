set(SOURCES
	src/compressors_test.cpp
)

set(HEADERS
	include/zlib_test.hpp
	include/snappy_test.hpp
	include/lzo_test.hpp
	include/bzip2_test.hpp
	include/lzma_test.hpp
)

add_executable(compressors_test ${SOURCES} ${HEADERS})
target_link_libraries(compressors_test
						test_utilities
						gtest
						zlib_library
						snappy
						lzo_library
						bzip2_library
						lzma_library
)

include_directories(include)