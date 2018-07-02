set(SOURCES
	io_common.cpp
	asynchronous_buffer.cpp
	synchronized_file.cpp
	performance_data_writer.cpp
	directory_explorer.cpp
)

add_library(io SHARED ${SOURCES})
target_link_libraries(io g3logger)
