/**
 *  AutoComp Socket Template
 *  socket.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/24/2018
 */

#ifndef AC_SOCKET_HPP
#define AC_SOCKET_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <netdb.h>
#include <cerrno>
#include <string>
#include <cstring>
#include <utility>        // std::move

#include "utils/exceptions.hpp"

namespace autocomp
{
  namespace net
  {

  /**
   * Socket template
   */
  class Socket
  {
  protected:

    unsigned short port;  //!< The port number the socket will be bound to
    int fileDescriptor;   //!< Socket's file descriptor
    int backlog;
    sockaddr_in address;

  public:

    /*
     * Instantiates a socket object with a port number
     *
     * @param port The port number the socket will be bound to
     * @param backlog The maximum number of incoming requests
     */
    Socket(const unsigned short & port = 0, const int & backlog = 10);

    Socket(Socket && other);

    Socket & operator=(Socket && other);

    Socket(const Socket &) = delete;
    Socket & operator=(const Socket &) = delete;

    virtual ~Socket();

    /*
     * Gets the port number
     */
    unsigned short getPort() const;

    /*
     * Gets the hostname of the socket
     *
     * @returns The hostname of the socket
     */
    std::string getHostname() const;

    /*
     * Gets the file descriptor
     */
    int getFileDescriptor() const;

    void close();

  protected:

    /*
     * Only to instantiate client sockets
     *
     * @param fileDescriptor Socket's file descriptor
     * @param address Socket's address
     */
    Socket(const int & fileDescriptor, const sockaddr_in & address);

    std::string getErrnoMessage() const;

  }; // class Socket

  } // namespace net
} // namespace autocomp

#endif // AC_SOCKET_HPP