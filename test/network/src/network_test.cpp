#include "gtest/gtest.h"

#include "tcp_socket_test.hpp"
//#include "server_test.hpp"
#include "client_server_test.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}