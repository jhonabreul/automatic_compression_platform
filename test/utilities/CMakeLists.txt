set(SOURCES
	src/common_functions.cpp
)

set(HEADERS
	include/common_functions.hpp
	include/test_constants.hpp
)

add_library(test_utilities ${SOURCES} ${HEADERS})

include_directories(include)