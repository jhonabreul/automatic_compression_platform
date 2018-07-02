set(SOURCES
    monitors.cpp
)

add_library(monitors STATIC ${SOURCES})
target_link_libraries(monitors
                      socket
                      utils
)