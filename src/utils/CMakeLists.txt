set(SOURCES
	buffer.cpp
	thread_pool.cpp
	decision_tree.cpp
)

add_library(utils SHARED ${SOURCES})

target_link_libraries(utils messaging)
