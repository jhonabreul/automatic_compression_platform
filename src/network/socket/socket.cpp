/**
 *  AutoComp Socket Template
 *  socket.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/24/2018
 */

#include "network/socket/socket.hpp"

namespace autocomp
{
  namespace net
  {

  // Instantiates a socket object with a port number
  Socket::Socket(const unsigned short & port, const int & backlog)
    : port(port),
      backlog(backlog)
  {
    this->fileDescriptor = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (this->fileDescriptor == -1) {
      throw exceptions::NetworkError(std::string("Error creating new socket: ")
                                      .append(this->getErrnoMessage()));
    }

    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(this->port);
  }

  // Instantiates a socket with a file descriptor and an address
  Socket::Socket(const int & fileDescriptor, const sockaddr_in & address)
    : fileDescriptor(fileDescriptor),
      address(address),
      port(ntohs(address.sin_port)),
      backlog(0)
  {}

  Socket::Socket(Socket && other)
    : port(0),
      backlog(0),
      fileDescriptor(-1)
  {
    *this = std::move(other);
  }

  Socket & Socket::operator=(Socket && other)
  {
    if (this != &other) {
      this->port = other.port;
      this->fileDescriptor = other.fileDescriptor;
      this->backlog = other.backlog;
      this->address = other.address;

      other.port = 0;
      other.fileDescriptor = -1;
      other.backlog = 10;
      other.address.sin_port = htons(other.port);
    }

    return *this;
  }

  Socket::~Socket()
  {
    this->close();
  }

  unsigned short Socket::getPort() const
  {
    return this->port;
  }

  // Gets the hostname of the socket
  std::string Socket::getHostname() const
  {
    char hostname[1025] = "";

    int resolveResult = ::getnameinfo((sockaddr *) &this->address,
                                      (socklen_t) sizeof(this->address),
                                      hostname, 1025, nullptr, 0, 0); 
    if (resolveResult != 0) {
      throw exceptions::NetworkError(std::string("Error resolving socket "
                                                 "address name. ")
                                      .append(gai_strerror(resolveResult))
                                      .append(" (error code: ")
                                      .append(std::to_string(resolveResult))
                                      .append(")"));
    }

    return std::string(hostname);
  }

  int Socket::getFileDescriptor() const
  {
    return this->fileDescriptor;
  }

  void Socket::close()
  {
    if (this->fileDescriptor != -1) {
      ::close(this->fileDescriptor);
      this->fileDescriptor = -1;
    }
  }

  std::string Socket::getErrnoMessage() const
  {
    return std::string(std::strerror(errno))
            .append(" (errno code: ")
            .append(std::to_string(errno))
            .append(")");
  }

  } // namespace net
} // namespace autocomp