#ifndef AC_SERVER_TEST_HPP
#define AC_SERVER_TEST_HPP

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <unistd.h>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "utils/exceptions.hpp"
#include "network/server/server.hpp"

class ServerTest : public ::testing::Test
{
protected:

  const std::string pingMessage = "PING";
  const std::string pongMessage = "PONG";
  const int nMessages = 10;
  const std::string shutdownPipeName = "/tmp/autocomp_shutdown.fifo";
  std::vector<std::thread> clients;

  ServerTest()
    : clients(50)
  {}

  void SetUp()
  {
    unlink(this->shutdownPipeName.c_str());
    ASSERT_EQ(0, ::mkfifo(this->shutdownPipeName.c_str(), 0600));
  }

public:

  void client()
  {
    autocomp::net::TCPSocket socket;
    
    ASSERT_NO_THROW({
      socket.connect("localhost", autocomp::test::constants::testPortThree);
    });

    std::string message(10, '\0');

    for (int i = 0; i < this->nMessages; i++) {
      message.clear();
      socket.send(this->pingMessage);
      socket.receive(message);
      ASSERT_EQ(this->pongMessage, message);
    }
  }

  void mainThread()
  {
    // Wait for server to be ready
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (auto & client : this->clients) {
      client = std::thread(&ServerTest::client, this);
    }

    for (auto & client : this->clients) {
      client.join();
    }

    int shutdownPipeFileDescriptor = ::open(this->shutdownPipeName.c_str(),
                                            O_WRONLY | O_NONBLOCK);

    if (shutdownPipeFileDescriptor == -1) {
      std::cout << "Error opening fifo: " <<  std::strerror(errno)
                << "(" << errno << ")" << std::endl;

      return;
    }

    ::write(shutdownPipeFileDescriptor, "1", 1);
    ::close(shutdownPipeFileDescriptor);
  }
}; // class ServerTest

TEST_F(ServerTest, PingPong)
{
  std::unique_ptr<autocomp::net::Server> server;

  try {
    server = std::unique_ptr<autocomp::net::Server>(
                new autocomp::net::Server(autocomp::test::constants
                                                        ::testPortThree,
                                          this->shutdownPipeName)
              );

    server->init();
  }
  catch (autocomp::exceptions::NetworkError & error) {
    std::cout << "Error initializing server: " << error.what() << std::endl;
  }

  std::thread mainThread(&ServerTest::mainThread, this);
  server->serve();
  mainThread.join();
}

#endif //AC_SERVER_TEST_HPP