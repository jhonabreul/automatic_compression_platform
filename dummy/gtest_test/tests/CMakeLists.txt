set(SOURCES
	test_all.cpp
)

set(HEADERS
	addition_test.h
	division_test.h
)

add_executable(test_mymath ${SOURCES} ${HEADERS})
target_link_libraries(test_mymath mymath gtest)