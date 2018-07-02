set(SOURCES
  src/network_test.cpp
)

set(HEADERS
  include/tcp_socket_test.hpp
  include/server_test.hpp
)

add_executable(network_test ${SOURCES} ${HEADERS})
target_link_libraries(network_test
                      test_utilities
                      gtest
                      utils
                      socket
                      server
                      client
)

include_directories(include)