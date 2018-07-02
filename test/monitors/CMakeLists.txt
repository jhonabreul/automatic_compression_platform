set(SOURCES
  cpu_monitor_test.cpp
)

add_executable(cpu_monitor ${SOURCES})
target_link_libraries(cpu_monitor
                      monitors
                      ${CMAKE_THREAD_LIBS_INIT}
)