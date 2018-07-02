set(SOURCES
  src/messaging_test.cpp
)

set(HEADERS
  include/messaging_test.hpp
)

add_executable(messagging_test ${SOURCES} ${HEADERS})
target_link_libraries(messagging_test
                      test_utilities
                      gtest
                      utils
                      messaging
)

include_directories(include)