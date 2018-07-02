set(SOURCES
  server.cpp
)

add_library(server STATIC ${SOURCES})
target_link_libraries(server
                      socket
                      compression
                      messaging
                      monitors
                      g3logger
                      ${CMAKE_THREAD_LIBS_INIT}
)
