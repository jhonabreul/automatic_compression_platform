set(SOURCES
    autocomp_server.cpp
)

add_executable(autocomp_server ${SOURCES})
target_link_libraries(autocomp_server
                      server
)