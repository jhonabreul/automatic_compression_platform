#ifndef AC_TCP_SOCKET_TEST_HPP
#define AC_TCP_SOCKET_TEST_HPP

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "utils/exceptions.hpp"
#include "network/socket/tcp_socket.hpp"

class TCPSocketTest : public ::testing::Test
{
protected:

  const std::string pingMessage = "PING";
  const std::string pongMessage = "PONG";
  const int nMessages = 20;

public:

  void serve()
  {
    autocomp::net::TCPSocket socket(autocomp::test::constants::testPortTwo);
    
    try {
      socket.bind();
    }
    catch(autocomp::exceptions::NetworkError & error) {
      std::cout << "Could not bind to the socket, try later" << std::endl;
      exit(0);
    }

    ASSERT_NO_THROW(socket.listen());
    
    std::shared_ptr<autocomp::net::TCPSocket> clientSocket;
    ASSERT_NO_THROW(clientSocket = socket.accept());

    std::string message(10, '\0');

    for (int i = 0; i < this->nMessages; i++) {
      message.clear();
      clientSocket->receive(message);
      ASSERT_EQ(this->pingMessage, message);
      clientSocket->send(this->pongMessage);
    }
  }

  void request()
  {
    autocomp::net::TCPSocket socket;
    
    ASSERT_NO_THROW({
      socket.connect("localhost", autocomp::test::constants::testPortTwo);
    });

    std::string message(10, '\0');

    for (int i = 0; i < this->nMessages; i++) {
      message.clear();
      socket.send(this->pingMessage);
      socket.receive(message);
      ASSERT_EQ(this->pongMessage, message);
    }
  }
}; // class TCPSocketTest

TEST_F(TCPSocketTest, ThrowsOnBusyPort)
{
  autocomp::net::TCPSocket socket(autocomp::test::constants::testPortOne);
  autocomp::net::TCPSocket secondSocket(autocomp::test::constants::testPortOne);

  try {
    socket.bind();
  }
  catch(autocomp::exceptions::NetworkError & error) {
    std::cout << "Could not bind to the socket, try later" << std::endl;
    exit(0);
  }

  ASSERT_THROW(secondSocket.bind(), autocomp::exceptions::NetworkError);
}

TEST_F(TCPSocketTest, PingPong)
{
  std::thread server(&TCPSocketTest::serve, this);
  usleep(500000);
  std::thread client(&TCPSocketTest::request, this);

  client.join();
  server.join();
}

#endif //AC_TCP_SOCKET_TEST_HPP