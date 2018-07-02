set(SOURCES
  src/utils_test.cpp
)

set(HEADERS
  include/buffer_test.hpp
  include/directory_explorer_test.hpp
  include/synchronous_queue_test.hpp
  include/thread_pool_test.hpp
)

add_executable(utils_test ${SOURCES} ${HEADERS})
target_link_libraries(utils_test
                      test_utilities
                      gtest
                      utils
                      io
)

set(BC_SOURCES
  src/bytecounting_test.cpp
)

add_executable(bytecounting_test ${BC_SOURCES})
target_link_libraries(bytecounting_test
                      test_utilities
                      utils
                      io
)

include_directories(include)