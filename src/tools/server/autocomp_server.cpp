/**
 *  AutoComp Server Executable
 *  autocomp_server.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/09/2018
 */

#include <iostream>
#include <thread>
#include <string>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cctype>

#include "utils/constants.hpp"
#include "network/server/server.hpp"

namespace 
{
  volatile std::sig_atomic_t closeoutInProgres = 0;

  int shutdownPipeFileDescriptor;
}

void usage(const std::string &);

void closeout(int);

int main(int argc, char * argv[])
{
  unsigned short port = autocomp::constants::DEFAULT_SERVER_PORT;
  unsigned int nThreads = std::thread::hardware_concurrency();
  int option;

  while ((option = getopt(argc, argv, "p:t:h?")) != -1) {
    switch (option) {
      case 'p':
        port = std::atoi(optarg);
        break;

      case 't':
        nThreads = std::atoi(optarg);
        break;

      case 'h':
      case '?':
        switch (optopt) {
          case 'p':
          case 't':
            std::cerr << "Option -" << (char) optopt
                      << " requires an argument\n";
            break;

          case 'h':
          case '?':
            break;

          default:
            std::cerr << "Unknown option -" << (char) optopt << "\n";
            break;
        }
        
        usage(argv[0]);

        std::exit(EXIT_SUCCESS);

      default:
        std::exit(EXIT_FAILURE);
    }
  }

  std::unique_ptr<autocomp::net::Server> server;

  try {
    server =
      std::unique_ptr<autocomp::net::Server>(
          new autocomp::net::Server(port, nThreads,
                                    autocomp::constants::SHUTDOWN_PIPE_NAME)
        );
  }
  catch (autocomp::exceptions::NetworkError & error) {
    std::cerr << "Could not create AutoComp server instance: " << error.what()
              << std::endl;

    std::exit(EXIT_FAILURE);
  }

  ::unlink(autocomp::constants::SHUTDOWN_PIPE_NAME.c_str());
  if (::mkfifo(autocomp::constants::SHUTDOWN_PIPE_NAME.c_str(), 0600) == -1) {
    std::cerr << "An error ocurred during server instantiation (mkfifo): "
              << std::strerror(errno) << std::endl;

    std::exit(EXIT_FAILURE);
  }

  try {
    server->init();
  }
  catch (autocomp::exceptions::NetworkError & error) {
    std::cerr << "Could not initialize AutoComp server: " << error.what()
              << std::endl;

    std::exit(EXIT_FAILURE);
  }

  shutdownPipeFileDescriptor =
    ::open(autocomp::constants::SHUTDOWN_PIPE_NAME.c_str(),
           O_WRONLY | O_NONBLOCK);

  if (shutdownPipeFileDescriptor == -1) {
    std::cerr << "An error ocurred during server instantiation (open): "
              << std::strerror(errno) << std::endl;

    std::exit(EXIT_FAILURE);
  }

  std::signal(SIGINT, closeout);
  std::signal(SIGTERM, closeout);
  std::signal(SIGQUIT, closeout);

  server->serve();
  
  return 0;
}

void usage(const std::string & binaryName)
{
  std::cerr << "usage: " << binaryName << " [-p port] [-t number_of_threads]\n";
}

void closeout(int signalNumber)
{
  if (closeoutInProgres) {
    std::raise(signalNumber);
  }

  closeoutInProgres = 1;

  ::write(shutdownPipeFileDescriptor, "1", 1);

  std::signal(SIGINT, SIG_IGN);
  std::signal(SIGTERM, SIG_IGN);
  std::signal(SIGQUIT, SIG_IGN);
}