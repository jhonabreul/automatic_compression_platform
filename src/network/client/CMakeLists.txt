set(SOURCES
    client.cpp
)

add_library(client STATIC ${SOURCES})
target_link_libraries(client
                      socket
                      compression
                      messaging
                      g3logger
)
