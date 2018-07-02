set(SOURCES
    autocomp_client.cpp
)

add_executable(autocomp ${SOURCES})
target_link_libraries(autocomp
                      client
                      ${CMAKE_THREAD_LIBS_INIT}
)