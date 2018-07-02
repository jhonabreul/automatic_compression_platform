#ifndef AC_CLIENT_SERVER_TEST_HPP
#define AC_CLIENT_SERVER_TEST_HPP

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "common_functions.hpp"
#include "test_constants.hpp"
#include "utils/exceptions.hpp"
#include "messaging/file_request_mode.pb.h"
#include "network/server/server.hpp"
#include "network/client/client.hpp"

class ClientServerTest : public ::testing::Test
{
protected:

  const int nMessages = 10;
  const std::string shutdownPipeName = "/tmp/autocomp_shutdown.fifo";
  std::vector<std::thread> clients;

  std::vector<std::string> realFileList{
    "test/test_files/alice29.txt",
    "test/test_files/test_dir/test_file2.txt",
    "test/test_files/test_dir/test_file3.xml",
    "test/test_files/test_dir/test_file.txt",
    "test/test_files/test_dir/test_subdir2/test_file.html",
    "test/test_files/test_dir/test_subdir2/test_subsubdir/test_subsubfile.txt",
    "test/test_files/test_dir/test_subdir/test_subfile.txt",
    "test/test_files/test.trace"
  };

  char stringBuffer[1000];

  pid_t currentServerPID;

  ClientServerTest()
    : clients(50)
  {}

  void SetUp()
  {
    unlink(this->shutdownPipeName.c_str());
    ASSERT_EQ(0, ::mkfifo(this->shutdownPipeName.c_str(), 0600));
  }

  void TearDown()
  {
    this->killServer();

    for (auto & absolutePath : this->realFileList) {
      ::strcpy(stringBuffer, absolutePath.c_str());
      std::string fileToRemove(autocomp::test::constants::testOutputDirectory +
                               "/" + ::basename(stringBuffer));

      ::remove(fileToRemove.c_str());
    }
  }

public:

  void server(const int & port)
  {
    std::unique_ptr<autocomp::net::Server> server;

    try {
      server =
        std::unique_ptr<autocomp::net::Server>(
            new autocomp::net::Server(port, this->shutdownPipeName)
          );

      server->init();
    }
    catch (autocomp::exceptions::NetworkError & error) {
      std::cout << "Error initializing server: " << error.what() << std::endl;
      std::exit(-1);
    }

    server->serve();
  }

  void killServer()
  {
    int shutdownPipeFileDescriptor = ::open(this->shutdownPipeName.c_str(),
                                            O_WRONLY | O_NONBLOCK);

    if (shutdownPipeFileDescriptor == -1) {
       FAIL() << "Error opening fifo: " <<  std::strerror(errno)
              << "(" << errno << ")" << std::endl;
    }

    ::write(shutdownPipeFileDescriptor, "1", 1);

    int status;
    ::waitpid(this->currentServerPID, &status, 0);

    ::close(shutdownPipeFileDescriptor);
  }
 
}; // class ClientServerTest


TEST_F(ClientServerTest, TransfersSingleFileWithSingleCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    this->server(autocomp::test::constants::testPortFour);

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::Compressor compressors[] = {
                                          autocomp::ZLIB,
                                          autocomp::SNAPPY,
                                          autocomp::LZO,
                                          autocomp::BZIP2,
                                          autocomp::LZMA,
                                          autocomp::FPC,
                                          autocomp::COPY
                                       };

  for (auto & compressor : compressors) {
    autocomp::net::Client client("localhost",
                                 autocomp::test::constants::testPortFour);
    
    client.init();

    autocomp::FileRequestMode mode = autocomp::COMPRESS;
    client.requestFile(autocomp::test::constants::fpcTestFilename, mode,
                       &compressor, nullptr,
                       autocomp::test::constants::testOutputDirectory);

    // Checking file integrity
    char sentFileName[1000 + 1];
    ::strncpy(sentFileName,
              autocomp::test::constants::fpcTestFilename.c_str(), 1000);
    std::string sentFile = autocomp::test::constants::testOutputDirectory +
                            "/" + ::basename(sentFileName);
    std::string originalFileData, sentFileData;

    ASSERT_NO_THROW({
      sentFileData = autocomp::test::getDataFromFile(sentFile);
    });

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(
          autocomp::test::constants::fpcTestFilename
        );
    });

    ASSERT_EQ(originalFileData.size(), sentFileData.size());
    ASSERT_TRUE(originalFileData == sentFileData);

    ::remove(sentFile.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_F(ClientServerTest, TransfersDirectoryWithSingleCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    ASSERT_NO_THROW({
      this->server(autocomp::test::constants::testPortFive);
    });

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::Compressor compressors[] = {
                                          autocomp::ZLIB,
                                          autocomp::SNAPPY,
                                          autocomp::LZO,
                                          autocomp::BZIP2,
                                          autocomp::LZMA,
                                          //autocomp::FPC,
                                          autocomp::COPY
                                       };

  for (auto & compressor : compressors) {
    autocomp::net::Client client("localhost",
                                 autocomp::test::constants::testPortFive);
    
    client.init();

    autocomp::FileRequestMode mode = autocomp::COMPRESS;
    client.requestFile(autocomp::test::constants::testDirectory, mode,
                       &compressor, nullptr,
                       autocomp::test::constants::testOutputDirectory);

    // Checking file integrity
    char sentFileName[1000 + 1];

    for (auto & file : this->realFileList) {
      ::strncpy(sentFileName, file.c_str(), 1000);
      std::string sentFile = autocomp::test::constants::testOutputDirectory +
                              "/" + ::basename(sentFileName);
      std::string originalFileData, sentFileData;

      ASSERT_NO_THROW({
        sentFileData =  autocomp::test::getDataFromFile(sentFile);
      });

      ASSERT_NO_THROW({
        originalFileData = autocomp::test::getDataFromFile(file);
      });

      ASSERT_EQ(originalFileData.size(), sentFileData.size());
      ASSERT_TRUE(originalFileData == sentFileData);

      ::remove(sentFile.c_str());
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_F(ClientServerTest, TransfersSingleFileWithAutoCompCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    this->server(autocomp::test::constants::testPortFour);

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFour);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::AUTOCOMP;
  client.requestFile(autocomp::test::constants::fpcTestFilename, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
  ::strncpy(sentFileName,
            autocomp::test::constants::fpcTestFilename.c_str(), 1000);
  std::string sentFile = autocomp::test::constants::testOutputDirectory +
                          "/" + ::basename(sentFileName);
  std::string originalFileData, sentFileData;

  ASSERT_NO_THROW({
    sentFileData = autocomp::test::getDataFromFile(sentFile);
  });

  ASSERT_NO_THROW({
    originalFileData = autocomp::test::getDataFromFile(
        autocomp::test::constants::fpcTestFilename
      );
  });

  ASSERT_EQ(originalFileData.size(), sentFileData.size());
  ASSERT_TRUE(originalFileData == sentFileData);

  ::remove(sentFile.c_str());

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(ClientServerTest, TransfersDirectoryWithAutoCompCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    ASSERT_NO_THROW({
      this->server(autocomp::test::constants::testPortFive);
    });

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFive);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::AUTOCOMP;
  client.requestFile(autocomp::test::constants::testDirectory, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
 
  for (auto & file : this->realFileList) {
    ::strncpy(sentFileName, file.c_str(), 1000);
    std::string sentFile = autocomp::test::constants::testOutputDirectory +
                            "/" + ::basename(sentFileName);
    std::string originalFileData, sentFileData;

    ASSERT_NO_THROW({
      sentFileData =  autocomp::test::getDataFromFile(sentFile);
    });

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(file);
    });

    ASSERT_EQ(originalFileData.size(), sentFileData.size());
    ASSERT_TRUE(originalFileData == sentFileData);

    ::remove(sentFile.c_str());
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

/*
TEST_F(ClientServerTest, TransfersSingleFileWithRoundRobinCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    this->server(autocomp::test::constants::testPortFour);

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFour);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::AUTOCOMP;
  client.requestFile(autocomp::test::constants::fpcTestFilename, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
  ::strncpy(sentFileName,
            autocomp::test::constants::fpcTestFilename.c_str(), 1000);
  std::string sentFile = autocomp::test::constants::testOutputDirectory +
                          "/" + ::basename(sentFileName);
  std::string originalFileData, sentFileData;

  ASSERT_NO_THROW({
    sentFileData = autocomp::test::getDataFromFile(sentFile);
  });

  ASSERT_NO_THROW({
    originalFileData = autocomp::test::getDataFromFile(
        autocomp::test::constants::fpcTestFilename
      );
  });

  ASSERT_EQ(originalFileData.size(), sentFileData.size());
  ASSERT_TRUE(originalFileData == sentFileData);

  ::remove(sentFile.c_str());

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(ClientServerTest, TransfersDirectoryWithRoundRobinCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    ASSERT_NO_THROW({
      this->server(autocomp::test::constants::testPortFive);
    });

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFive);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::AUTOCOMP;
  client.requestFile(autocomp::test::constants::testDirectory, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
 
  for (auto & file : this->realFileList) {
    ::strncpy(sentFileName, file.c_str(), 1000);
    std::string sentFile = autocomp::test::constants::testOutputDirectory +
                            "/" + ::basename(sentFileName);
    std::string originalFileData, sentFileData;

    ASSERT_NO_THROW({
      sentFileData =  autocomp::test::getDataFromFile(sentFile);
    });

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(file);
    });

    ASSERT_EQ(originalFileData.size(), sentFileData.size());
    ASSERT_TRUE(originalFileData == sentFileData);

    ::remove(sentFile.c_str());
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

*/

TEST_F(ClientServerTest, TransfersSingleFileWithoutCompressing)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    this->server(autocomp::test::constants::testPortFour);

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFour);
    
  client.init();

  autocomp::FileRequestMode mode = autocomp::NO_COMPRESSION;
  client.requestFile(autocomp::test::constants::fpcTestFilename, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
  ::strncpy(sentFileName,
            autocomp::test::constants::fpcTestFilename.c_str(), 1000);
  std::string sentFile = autocomp::test::constants::testOutputDirectory +
                          "/" + ::basename(sentFileName);
  std::string originalFileData, sentFileData;

  ASSERT_NO_THROW({
    sentFileData = autocomp::test::getDataFromFile(sentFile);
  });

  ASSERT_NO_THROW({
    originalFileData = autocomp::test::getDataFromFile(
        autocomp::test::constants::fpcTestFilename
      );
  });

  ASSERT_EQ(originalFileData.size(), sentFileData.size());
  ASSERT_TRUE(originalFileData == sentFileData);

  ::remove(sentFile.c_str());

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(ClientServerTest, TransfersDirectoryWithoutCompressing)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    ASSERT_NO_THROW({
      this->server(autocomp::test::constants::testPortFive);
    });

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFive);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::NO_COMPRESSION;
  client.requestFile(autocomp::test::constants::testDirectory, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
 
  for (auto & file : this->realFileList) {
    ::strncpy(sentFileName, file.c_str(), 1000);
    std::string sentFile = autocomp::test::constants::testOutputDirectory +
                            "/" + ::basename(sentFileName);
    std::string originalFileData, sentFileData;

    ASSERT_NO_THROW({
      sentFileData =  autocomp::test::getDataFromFile(sentFile);
    });

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(file);
    });

    ASSERT_EQ(originalFileData.size(), sentFileData.size());
    ASSERT_TRUE(originalFileData == sentFileData);

    ::remove(sentFile.c_str());
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(ClientServerTest, TransfersSingleFileByPreCompressing)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    this->server(autocomp::test::constants::testPortFour);

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::Compressor compressors[] = {
                                          autocomp::ZLIB,
                                          //autocomp::SNAPPY,
                                          autocomp::LZO,
                                          autocomp::BZIP2,
                                          autocomp::LZMA,
                                          autocomp::FPC,
                                          autocomp::COPY
                                       };

  for (auto & compressor : compressors) {
    autocomp::net::Client client("localhost",
                                 autocomp::test::constants::testPortFour);
    
    client.init();

    autocomp::FileRequestMode mode = autocomp::FileRequestMode::PRE_COMPRESS;
    client.requestFile(autocomp::test::constants::fpcTestFilename, mode,
                       &compressor, nullptr,
                       autocomp::test::constants::testOutputDirectory);

    // Checking file integrity
    char sentFileName[1000 + 1];
    ::strncpy(sentFileName,
              autocomp::test::constants::fpcTestFilename.c_str(), 1000);
    std::string sentFile = autocomp::test::constants::testOutputDirectory +
                            "/" + ::basename(sentFileName);
    std::string originalFileData, sentFileData;

    ASSERT_NO_THROW({
      sentFileData = autocomp::test::getDataFromFile(sentFile);
    });

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(
          autocomp::test::constants::fpcTestFilename
        );
    });

    ASSERT_EQ(originalFileData.size(), sentFileData.size());
    ASSERT_TRUE(originalFileData == sentFileData);

    ::remove(sentFile.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_F(ClientServerTest, TransfersDirectoryByPreCompressing)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    ASSERT_NO_THROW({
      this->server(autocomp::test::constants::testPortFive);
    });

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::Compressor compressors[] = {
                                          autocomp::ZLIB,
                                          //autocomp::SNAPPY,
                                          autocomp::LZO,
                                          autocomp::BZIP2,
                                          autocomp::LZMA,
                                          //autocomp::FPC,
                                          autocomp::COPY
                                       };

  for (auto & compressor : compressors) {
    autocomp::net::Client client("localhost",
                                 autocomp::test::constants::testPortFive);
    
    client.init();

    autocomp::FileRequestMode mode = autocomp::PRE_COMPRESS;
    client.requestFile(autocomp::test::constants::testDirectory, mode,
                       &compressor, nullptr,
                       autocomp::test::constants::testOutputDirectory);

    // Checking file integrity
    char sentFileName[1000 + 1];

    for (auto & file : this->realFileList) {
      ::strncpy(sentFileName, file.c_str(), 1000);
      std::string sentFile = autocomp::test::constants::testOutputDirectory +
                              "/" + ::basename(sentFileName);
      std::string originalFileData, sentFileData;

      ASSERT_NO_THROW({
        sentFileData =  autocomp::test::getDataFromFile(sentFile);
      });

      ASSERT_NO_THROW({
        originalFileData = autocomp::test::getDataFromFile(file);
      });

      ASSERT_EQ(originalFileData.size(), sentFileData.size());
      ASSERT_TRUE(originalFileData == sentFileData);

      ::remove(sentFile.c_str());
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_F(ClientServerTest, TransfersSingleFileWithTrainingCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    this->server(autocomp::test::constants::testPortFour);

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFour);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::TRAIN;
  client.requestFile(autocomp::test::constants::fpcTestFilename, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
  ::strncpy(sentFileName,
            autocomp::test::constants::fpcTestFilename.c_str(), 1000);
  std::string sentFile = autocomp::test::constants::testOutputDirectory +
                          "/" + ::basename(sentFileName);
  std::string originalFileData, sentFileData;

  ASSERT_NO_THROW({
    sentFileData = autocomp::test::getDataFromFile(sentFile);
  });

  ASSERT_NO_THROW({
    originalFileData = autocomp::test::getDataFromFile(
        autocomp::test::constants::fpcTestFilename
      );
  });

  ASSERT_EQ(originalFileData.size(), sentFileData.size());
  ASSERT_TRUE(originalFileData == sentFileData);

  ::remove(sentFile.c_str());

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(ClientServerTest, TransfersDirectoryWithTrainingCompressor)
{
  this->currentServerPID = fork();

  //child
  if (this->currentServerPID == 0) {
    ASSERT_NO_THROW({
      this->server(autocomp::test::constants::testPortFive);
    });

    std::exit(0);
  }
  else if (this->currentServerPID == -1) {
    std::cerr << "Could not create child process: " << std::strerror(errno)
              << " (" << errno << ")" << std::endl;
    std::exit(-1);
  }

  // Wait for server to be ready
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (::kill(this->currentServerPID, 0) == -1 and errno == ESRCH) {
    FAIL() << "Failed to launch server" << std::endl;
  }

  autocomp::net::Client client("localhost",
                               autocomp::test::constants::testPortFive);
    
  client.init();
  autocomp::FileRequestMode mode = autocomp::TRAIN;
  client.requestFile(autocomp::test::constants::testDirectory, mode,
                     nullptr, nullptr,
                     autocomp::test::constants::testOutputDirectory);

  // Checking file integrity
  char sentFileName[1000 + 1];
 
  for (auto & file : this->realFileList) {
    ::strncpy(sentFileName, file.c_str(), 1000);
    std::string sentFile = autocomp::test::constants::testOutputDirectory +
                            "/" + ::basename(sentFileName);
    std::string originalFileData, sentFileData;

    ASSERT_NO_THROW({
      sentFileData =  autocomp::test::getDataFromFile(sentFile);
    });

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(file);
    });

    ASSERT_EQ(originalFileData.size(), sentFileData.size());
    ASSERT_TRUE(originalFileData == sentFileData);

    ::remove(sentFile.c_str());
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

#endif //AC_CLIENT_SERVER_TEST_HPP