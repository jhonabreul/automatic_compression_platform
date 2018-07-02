/**
 *  AutoComp TCP Socket Template
 *  tcp_socket.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/24/2018
 */

#ifndef AC_TCP_SOCKET_HPP
#define AC_TCP_SOCKET_HPP

#include <memory>
#include <vector>
#include <cstring>
#include <utility>
#include <chrono>
#include <mutex>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "utils/buffer.hpp"
#include "network/socket/socket.hpp"

namespace autocomp
{
  namespace net
  {

  /**
   * TPC Socket template
   */
  class TCPSocket : public Socket
  {
    mutable
    double lastBandwidth;
    mutable int previousBytesInSendBuffer;
    mutable std::chrono::time_point<std::chrono::high_resolution_clock>
      previousMeasureTime;
    mutable std::mutex bandwidthMutex;

  public:

    /*
     * Instantiates a TCP socket object with a port number
     *
     * @param port The port number the socket will be bound to
     * @param backlog The maximum number of incoming requests
     */
    TCPSocket(const unsigned short & port = 0, const int & backlog = 10);

    TCPSocket(TCPSocket && other);

    TCPSocket & operator=(TCPSocket && other);

    TCPSocket(const TCPSocket &) = delete;
    TCPSocket & operator=(const TCPSocket &) = delete;

    ~TCPSocket() {};

    void setSendBufferCapacity(const int & newCapacity);

    int getSendBufferCapacity() const;

    int getSendBufferSize() const;

    double getBandwidth() const;

    /*
     * Binds the socket to its configured port
     */
    void bind();

    /*
     * Makes the socket listen in the bound port
     */
    void listen();

    /*
     * Accepts an incoming connection
     *
     * @returns A pointer to the client socket
     */
    std::shared_ptr<TCPSocket> accept();

    /*
     * Connects to a socket
     */
    void connect(const std::string & hostname, const unsigned short & port);

    /*
     * Sends the data in the message object.
     * The protocol is as follows: the function sends the number of bytes
     * that message has. Then, the message itself is sent. 
     *
     * @param message The message to send
     *
     * @returns The number if bytes sent
     */
    std::size_t send(const std::string & message) const;

    /*
     * Sends the data in the message object.
     * The protocol is as follows: the function sends the number of bytes
     * that message has. Then, the message itself is sent. 
     *
     * @param message The message to send
     *
     * @returns The number if bytes sent
     */
    std::size_t send(const std::vector<char> & message) const;

    /*
     * Sends the data in the message object.
     * The protocol is as follows: the function sends the number of bytes
     * that message has. Then, the message itself is sent. 
     *
     * @param message The message to send
     *
     * @returns The number if bytes sent
     */
    std::size_t send(const Buffer & message) const;

    /*
     * Receives data from the sender.
     * The protocol is as follows: the function reads the number of bytes
     * that the incoming message has. Then, the message itself is read. 
     *
     * @param message A buffer where the message is going to be writen.
     *
     * @returns The number if bytes read
     */
    std::size_t receive(std::string & message) const;

    /*
     * Receives data from the sender.
     * The protocol is as follows: the function reads the number of bytes
     * that the incoming message has. Then, the message itself is read. 
     *
     * @param message A buffer where the message is going to be writen.
     *
     * @returns The number if bytes read
     */
    std::size_t receive(std::vector<char> & message) const;

    /*
     * Receives data from the sender.
     * The protocol is as follows: the function reads the number of bytes
     * that the incoming message has. Then, the message itself is read. 
     *
     * @param message A buffer where the message is going to be writen.
     *
     * @returns The number if bytes read
     */
    std::size_t receive(Buffer & message) const;

  private:

    /*
     * Instantiates a TCP socket object with an address
     *
     * @param fileDescriptor Socket's file descriptor
     * @param address Socket's address
     */
    TCPSocket(const int & fileDescriptor, const sockaddr_in & address);

    /*
     * Sends the size of the next message to transmit
     *
     * @param messageSize The size of the next message to transmit
     */
    void sendMessageSize(const uint32_t & messageSize) const;

    /*
     * Receives the size of the next message to read
     *
     * @returns The size of the next message to read
     */
    uint32_t receiveMessageSize() const;

    /*
     * Sends the data in the message object.
     * The protocol is as follows: the function sends the number of bytes
     * that message has. Then, the message itself is sent. 
     *
     * @param message The port number the socket will be bound to
     * @param bytesToSend The amount of bytes to send.
     *
     * @returns The number of bytes sent
     */
    std::size_t _send(const char * buffer, std::size_t bytesToSend) const;

    /*
     * Sends the data in the message object.
     * The protocol is as follows: the function sends the number of bytes
     * that message has. Then, the message itself is sent. 
     *
     * @param message The port number the socket will be bound to
     * @param bytesToRead The amount of bytes to receive
     *
     * @returns The number of bytes read
     */
    std::size_t _receive(char * buffer, std::size_t bytesToRead) const;

    void registerSendBufferAddition(const std::size_t & bytes) const;

    void estimateBandwidth() const;

  }; // class TCPSocket

  } // namespace net
} // namespace autocomp

#endif // AC_TCP_SOCKET_HPP