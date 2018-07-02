/**
 *  AutoComp TCP Socket Template
 *  tcp_socket.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/24/2018
 */

#include "network/socket/tcp_socket.hpp"

namespace autocomp
{
  namespace net
  {

  // Instantiates a TCP socket object with a port numb
  TCPSocket::TCPSocket(const unsigned short & port, const int & backlog)
    : Socket(port, backlog),
      previousBytesInSendBuffer(0),
      previousMeasureTime(std::chrono::high_resolution_clock::now())
  {}

  // Instantiates a TCP socket object with an address
  TCPSocket::TCPSocket(const int & fileDescriptor, const sockaddr_in & address)
    : Socket(fileDescriptor, address),
      previousBytesInSendBuffer(0),
      previousMeasureTime(std::chrono::high_resolution_clock::now())
  {}

  TCPSocket::TCPSocket(TCPSocket && other)
  {
    *this = std::move(other);    
  }

  TCPSocket & TCPSocket::operator=(TCPSocket && other)
  {
    std::swap(this->lastBandwidth, other.lastBandwidth);
    std::swap(this->previousBytesInSendBuffer,
              other.previousBytesInSendBuffer);
    std::swap(this->previousMeasureTime, other.previousMeasureTime);
    Socket::operator=(std::move(other));

    return *this;
  }

  void TCPSocket::registerSendBufferAddition(const std::size_t & bytes) const
  {
    this->previousMeasureTime = std::chrono::high_resolution_clock::now();
    this->previousBytesInSendBuffer = this->getSendBufferSize();
  }

  void TCPSocket::setSendBufferCapacity(const int & newCapacity)
  {
    if (::setsockopt(this->fileDescriptor, SOL_SOCKET, SO_SNDBUF, &newCapacity,
                     sizeof(newCapacity)) == -1) {
      throw exceptions::NetworkError(std::string("Error setting socket's send"
                                                 " buffer's capacity to ")
                                         .append(std::to_string(newCapacity))
                                         .append(": ")
                                         .append(this->getErrnoMessage()));
    }
  }

  int TCPSocket::getSendBufferCapacity() const
  {
    int sendBufferCapacity;
    socklen_t sendBufferCapacityLength = sizeof(sendBufferCapacity);

    if (::getsockopt(this->fileDescriptor, SOL_SOCKET, SO_SNDBUF,
                     &sendBufferCapacity, &sendBufferCapacityLength) == -1) {
      throw exceptions::NetworkError(std::string("Error obtaining socket's send"
                                                 " buffer's capacity: ")
                                         .append(this->getErrnoMessage()));
    }

    return sendBufferCapacity;
  }

  int TCPSocket::getSendBufferSize() const
  {
    static std::mutex mutex;
    int sendBufferSize;

    //std::unique_lock<std::mutex> guard(mutex);

    if (::ioctl(this->fileDescriptor, SIOCOUTQ, &sendBufferSize) == -1) {
      throw exceptions::NetworkError(std::string("Error obtaining socket's send"
                                                 " buffer's size: ")
                                         .append(this->getErrnoMessage()));
    }

    return sendBufferSize;
  }

  double TCPSocket::getBandwidth() const
  {
    std::unique_lock<std::mutex> lock(this->bandwidthMutex);
    return this->lastBandwidth;
  }

  // Binds the socket to its configured port
  void TCPSocket::bind()
  {
    if (this->fileDescriptor == -1) {
      return;
    }

    int bindResult = ::bind(this->fileDescriptor, (sockaddr *) &this->address,
                            sizeof(this->address));

    if (bindResult == -1) {
      throw exceptions::NetworkError(std::string("Error binding socket to "
                                                 "port ")
                                      .append(std::to_string(this->port))
                                      .append(": ")
                                      .append(this->getErrnoMessage()));
    }
  }

  // Makes the socket listen in the bound port
  void TCPSocket::listen()
  {
    if (this->fileDescriptor == -1) {
      return;
    }

    int listenResult = ::listen(this->fileDescriptor, this->backlog);

    if (listenResult == -1) {
      throw exceptions::NetworkError(std::string("Error making the socket "
                                                 "bound to the port")
                                      .append(std::to_string(this->port))
                                      .append(" listen for incoming "
                                              "connections: ")
                                      .append(this->getErrnoMessage()));
    }
  }

  // Accepts an incoming connection
  std::shared_ptr<TCPSocket> TCPSocket::accept()
  {
    sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    int clientFileDescriptor = ::accept(this->fileDescriptor,
                                        (sockaddr *) &clientAddress,
                                        &clientAddressLength);

    if (clientFileDescriptor == -1) {
      throw exceptions::NetworkError(std::string("Error accepting incoming "
                                                 "connection in socket bound "
                                                 "to the port ")
                                      .append(std::to_string(this->port))
                                      .append(": ")
                                      .append(this->getErrnoMessage()));
    }

    return std::shared_ptr<TCPSocket>(new TCPSocket(clientFileDescriptor,
                                                    clientAddress));
  }

  // Connects to a socket
  void TCPSocket::connect(const std::string & hostname,
                          const unsigned short & port)
  {
    addrinfo * remotePosibleAddresses, remoteAddressHints;

    std::memset(&remoteAddressHints, 0, sizeof(remoteAddressHints));
    remoteAddressHints.ai_family = AF_INET;
    remoteAddressHints.ai_socktype = SOCK_STREAM; // Only want stream-based
                                                  // connection

    // Resolve host name
    int resolveResult = ::getaddrinfo(hostname.c_str(),
                                      std::to_string(port).c_str(),
                                      &remoteAddressHints,
                                      &remotePosibleAddresses);

    if (resolveResult != 0) {
      throw exceptions::NetworkError(std::string("Error resolving hostname ")
                                      .append(hostname)
                                      .append(" address. ")
                                      .append(gai_strerror(resolveResult))
                                      .append(" (error code: ")
                                      .append(std::to_string(resolveResult))
                                      .append(")"));
    }

    sockaddr_in * remoteAddress = (sockaddr_in *)
                                    remotePosibleAddresses->ai_addr;
    remoteAddress->sin_port = htons(port);

    int connectResult = ::connect(this->fileDescriptor,
                                  (sockaddr *) remoteAddress,
                                  sizeof(*remoteAddress));

    ::freeaddrinfo(remotePosibleAddresses);

    if (connectResult == -1) {
      throw exceptions::NetworkError(std::string("Error connecting to the "
                                                 "address ")
                                      .append(hostname)
                                      .append(":")
                                      .append(std::to_string(port))
                                      .append(": ")
                                      .append(this->getErrnoMessage()));
    }
  }

  // Sends the data in the message object.
  // The protocol is as follows: the function sends the number of bytes
  // that message has. Then, the message itself is sent
  std::size_t TCPSocket::send(const std::string & message) const
  {
    this->sendMessageSize(message.size());
    //this->estimateBandwidth();

    std::size_t bytesSent = this->_send(message.data(), message.size());

    //this->registerSendBufferAddition(bytesSent);

    return bytesSent;
  }

  // Sends the data in the message object.
  // The protocol is as follows: the function sends the number of bytes
  // that message has. Then, the message itself is sent
  std::size_t TCPSocket::send(const std::vector<char> & message) const
  {
    this->sendMessageSize(message.size());
    //this->estimateBandwidth();

    std::size_t bytesSent = this->_send(message.data(), message.size());
    
    //this->registerSendBufferAddition(bytesSent);

    return bytesSent;
  }

  // Sends the data in the message object.
  // The protocol is as follows: the function sends the number of bytes
  // that message has. Then, the message itself is sent
  std::size_t TCPSocket::send(const Buffer & message) const
  {
    this->sendMessageSize(message.getSize());
    //this->estimateBandwidth();

    std::size_t bytesSent = this->_send(message.getData(), message.getSize());
    
    //this->registerSendBufferAddition(bytesSent);

    return bytesSent;
  }

  // Receives data from the sender.
  // The protocol is as follows: the function reads the number of bytes
  // that the incoming message has. Then, the message itself is read
  std::size_t TCPSocket::receive(std::string & message) const
  {
    std::vector<char> buffer;

    std::size_t bytesRead = this->receive(buffer);

    if (message.capacity() < bytesRead) {
      message.resize(bytesRead);
    }

    message.assign(buffer.begin(), buffer.end());

    return bytesRead;
  }

  // Receives data from the sender.
  // The protocol is as follows: the function reads the number of bytes
  // that the incoming message has. Then, the message itself is read
  std::size_t TCPSocket::receive(std::vector<char> & message) const
  {
    std::size_t messageSize = this->receiveMessageSize();

    //if (message.capacity() < messageSize) {
      message.resize(messageSize);
    //}

    return this->_receive(message.data(), messageSize);
  }

  // Receives data from the sender.
  // The protocol is as follows: the function reads the number of bytes
  // that the incoming message has. Then, the message itself is read
  std::size_t TCPSocket::receive(Buffer & message) const
  {
    std::size_t messageSize = this->receiveMessageSize();

    if (message.getCapacity() < messageSize) {
      message.resize(messageSize);
    }

    std::size_t bytesRead = this->_receive(message.getData(), messageSize);
    message.setSize(bytesRead);

    return bytesRead;
  }

  // Sends the size of the next message to transmit
  void TCPSocket::sendMessageSize(const uint32_t & messageSize) const
  {
    uint32_t networkByteOrderMessageSize = htonl(messageSize);
    char * buffer = (char *) &networkByteOrderMessageSize;

    this->_send(buffer, sizeof(networkByteOrderMessageSize));
  }

  // Receives the size of the next message to read
  uint32_t TCPSocket::receiveMessageSize() const
  {
    uint32_t networkByteOrderMessageSize;
    char * buffer = (char *) &networkByteOrderMessageSize;
    
    this->_receive(buffer, sizeof(networkByteOrderMessageSize));

    return ntohl(networkByteOrderMessageSize);
  }

  // Sends the data in the message object.
  // The protocol is as follows: the function sends the number of bytes
  // that message has. Then, the message itself is sent
  std::size_t
  TCPSocket::_send(const char * buffer, std::size_t bytesToSend) const
  {
    std::size_t bytesSent;
    std::size_t bytesLeftToSend = bytesToSend;

    do {
      bytesSent = write(this->fileDescriptor, buffer, bytesLeftToSend);

      if (bytesSent == -1) {
        throw exceptions::NetworkError(std::string("Error sending data to ")
                                        .append(this->getHostname())
                                        .append(":")
                                        .append(std::to_string(this->port))
                                        .append(": ")
                                        .append(this->getErrnoMessage()));
      }

      buffer += bytesSent;
      bytesLeftToSend -= bytesSent;
    } while (bytesLeftToSend > 0);

    return bytesToSend;
  }

  // Sends the data in the message object.
  // The protocol is as follows: the function sends the number of bytes
  // that message has. Then, the message itself is sent
  std::size_t TCPSocket::_receive(char * buffer, std::size_t bytesToRead) const
  {
    std::size_t bytesRead;
    std::size_t bytesLeftToRead = bytesToRead;

    do {
      bytesRead = read(this->fileDescriptor, buffer, bytesLeftToRead);

      switch (bytesRead) {
        default:
          buffer += bytesRead;
          bytesLeftToRead -= bytesRead;
          break;

        case -1:
          throw exceptions::NetworkError(std::string("Error receiving data "
                                                     "from ")
                                          .append(this->getHostname())
                                          .append(":")
                                          .append(std::to_string(this->port))
                                          .append(": ")
                                          .append(this->getErrnoMessage()));
          break;
      
        case 0:
          throw exceptions::NetworkError(std::string("Error receiving data "
                                                     "from ")
                                          .append(this->getHostname())
                                          .append(":")
                                          .append(std::to_string(this->port))
                                          .append(": the peer has performed an "
                                                  "orderly shutdown"));
          break;
      }
    } while (bytesLeftToRead > 0);

    return bytesToRead;
  }

  void TCPSocket::estimateBandwidth() const
  {
    auto currentTime = std::chrono::high_resolution_clock::now();
    int currentBytesInSendBuffer = this->getSendBufferSize();

    double elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            currentTime - this->previousMeasureTime
                          ).count();
    double bandwidth;

    //std::cout << elapsedTime << std::endl;

    if (elapsedTime != 0) {
      // 8E3 = 8E-6 (Mbits in byte ) / 1E-9 (seconds in a nanosecond)
      std::cout << previousBytesInSendBuffer << " - " << currentBytesInSendBuffer << " in " << 1E-9 * elapsedTime << std::endl;
      bandwidth =
        8E3 * (this->previousBytesInSendBuffer - currentBytesInSendBuffer) /
          elapsedTime;

      std::unique_lock<std::mutex> lock(this->bandwidthMutex);
      this->lastBandwidth = bandwidth;
    }
  }

  } // namespace net
} // namespace autocomp