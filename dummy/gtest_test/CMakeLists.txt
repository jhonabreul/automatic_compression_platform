set(SOURCES
	src/addition.cpp
	src/division.cpp
)

set(HEADERS
	include/addition.h
	include/division.h
)

add_library(mymath ${SOURCES} ${HEADERS})

include_directories(include)

################################################################################

add_subdirectory(tests)