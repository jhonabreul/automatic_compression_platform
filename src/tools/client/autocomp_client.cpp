/**
 *  AutoComp Client Executable
 *  autocomp_client.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/11/2018
 */

#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cctype>

#include "utils/constants.hpp"
#include "network/client/client.hpp"
#include "messaging/compressor.pb.h"
#include "messaging/file_request_mode.pb.h"

namespace 
{
  volatile std::sig_atomic_t closeoutInProgres = 0;
}

void usage(const std::string &);

void closeout(int);

int main(int argc, char * argv[])
{
  std::string hostname;
  unsigned short port = autocomp::constants::DEFAULT_SERVER_PORT;
  std::string requestedPath, destinationDirectory;
  std::unique_ptr<autocomp::Compressor> compressor;
  std::unique_ptr<int> compressionLevel;
  autocomp::FileRequestMode mode = autocomp::AUTOCOMP;

  int option;
  bool compressMode = false;
  bool precompressMode = false;

  while ((option = getopt(argc, argv, "f:d:m:c:l:H:P:h?")) != -1) {
    switch (option) {
      case 'H':
        hostname = optarg;
        break;

      case 'P':
        port = std::atoi(optarg);
        break;

      case 'f':
        requestedPath = optarg;
        break;

      case 'd':
        destinationDirectory = optarg;
        break;

      case 'm':
      {
        std::string modeName(optarg);
        std::transform(modeName.begin(), modeName.end(), modeName.begin(),
                       ::toupper);
        if (not FileRequestMode_Parse(modeName, &mode)) {
          std::cerr << "Invalid file request mode " << optarg << std::endl;
          std::exit(EXIT_FAILURE);
        }
        break;
      }

      case 'c':
      {
        std::string compressorName(optarg);
        autocomp::Compressor tmpCompressor;
        std::transform(compressorName.begin(), compressorName.end(),
                       compressorName.begin(), ::toupper);

        if (not autocomp::Compressor_Parse(compressorName, &tmpCompressor)) {
          std::cerr << "Invalid compressor " << optarg << std::endl;
          std::exit(EXIT_FAILURE);
        }

        compressor = std::unique_ptr<autocomp::Compressor>(
                        new autocomp::Compressor(tmpCompressor)
                      );
        break;
      }

      case 'l':
        compressionLevel = std::unique_ptr<int>(new int(std::atoi(optarg)));
        break;

      case 'h':
        usage(argv[0]);
        std::exit(EXIT_SUCCESS);

      case '?':
        switch (optopt) {
          case 'f':
          case 'd':
          case 'c':
          case 'l':
            std::cerr << "Option -" << (char) optopt
                      << " requires an argument\n";
            break;

          case '?':
            break;

          default:
            std::cerr << "Unknown option -" << (char) optopt << std::endl;
            break;
        }

        usage(argv[0]);

        std::exit(EXIT_SUCCESS);

      default:
        std::exit(EXIT_FAILURE);
    }
  }

  // <--- Host validation ---> //
  if (hostname.empty() or requestedPath.empty() or
      destinationDirectory.empty()) {
    std::cerr << "A mandatory argument was not given" << std::endl;
    usage(argv[0]);

    std::exit(EXIT_FAILURE);
  }

  autocomp::net::Client client(hostname, port);

  try {
    client.init();
  }
  catch (autocomp::exceptions::NetworkError & error) {
    std::cerr << "Could not initialize AutoComp client: " << error.what()
              << std::endl;

    std::exit(EXIT_FAILURE);
  }

  std::cout << "Requesting " << requestedPath << " to "
            << hostname << ":" << port << " ..." << std::endl;

  try {
    client.requestFile(requestedPath, mode, compressor.get(),
                       compressionLevel.get(), destinationDirectory);
  }
  catch (autocomp::exceptions::NetworkError & error) {
    std::cerr << "Could not receive the whole data: " << error.what()
              << std::endl;

    std::exit(EXIT_FAILURE);
  }
  
  return 0;
}

void usage(const std::string & binaryName)
{
  std::cerr << "usage: " << binaryName
            << "-H hostname [-P port] -f requested_path_or_file "
            << "-d destination_directory [-m file_request_mode] "
            << "[-c compressor_name] [-l compression_level]\n";
}

void closeout(int signalNumber)
{
  if (closeoutInProgres) {
    std::raise(signalNumber);
  }

  closeoutInProgres = 1;

  //::write(shutdownPipeFileDescriptor, "1", 1);

  std::signal(SIGINT, SIG_IGN);
  std::signal(SIGTERM, SIG_IGN);
  std::signal(SIGQUIT, SIG_IGN);
}