#ifndef AC_MESSAGING_TEST_HPP
#define AC_MESSAGING_TEST_HPP

/* C++ System Headers */
#include <vector>

/* External headers */
#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"

/* Project headers */
#include "utils/protobuf_utils.hpp"
#include "messaging/compressor.pb.h"
#include "messaging/file_request_mode.pb.h"
#include "messaging/error_message.pb.h"
#include "messaging/chunk_header.pb.h"
#include "messaging/file_initial_message.pb.h"
#include "messaging/file_transmission_request.pb.h"

TEST(MessagingTest, FileTransmissionRequestTest)
{
  autocomp::messaging::FileTransmissionRequest fileTransmissionRequest,
                                          recoveredFileTransmissionRequest;
  fileTransmissionRequest.set_filename("filename_1");
  fileTransmissionRequest.set_mode(autocomp::AUTOCOMP);
  fileTransmissionRequest.set_compressor(autocomp::LZO);
  fileTransmissionRequest.set_compressionlevel(5);

  std::vector<char> buffer;

  ASSERT_TRUE(autocomp::serializeMessage(fileTransmissionRequest, buffer));
  ASSERT_TRUE(autocomp::deserializeMessage(buffer,
                                           recoveredFileTransmissionRequest));

  ASSERT_EQ(fileTransmissionRequest.filename(),
            recoveredFileTransmissionRequest.filename());
  ASSERT_EQ(fileTransmissionRequest.mode(),
            recoveredFileTransmissionRequest.mode());
  ASSERT_EQ(fileTransmissionRequest.compressor(),
            recoveredFileTransmissionRequest.compressor());
  ASSERT_EQ(fileTransmissionRequest.compressionlevel(),
            recoveredFileTransmissionRequest.compressionlevel());
  ASSERT_TRUE(google::protobuf::util
              ::MessageDifferencer::Equals(fileTransmissionRequest,
                                         recoveredFileTransmissionRequest));
}

TEST(MessagingTest, FileInitialMessageTest)
{
  autocomp::messaging::FileInitialMessage fileInitialMessage,
                                          recoveredFileInitialMessage;
  fileInitialMessage.set_filename("filename_1");
  fileInitialMessage.set_filesize(9999);
  fileInitialMessage.set_chunksize(500);
  fileInitialMessage.set_lastfile(true);

  std::vector<char> buffer;

  ASSERT_TRUE(autocomp::serializeMessage(fileInitialMessage, buffer));
  ASSERT_TRUE(autocomp::deserializeMessage(buffer,
                                           recoveredFileInitialMessage));

  ASSERT_EQ(fileInitialMessage.filename(),
            recoveredFileInitialMessage.filename());
  ASSERT_EQ(fileInitialMessage.filesize(),
            recoveredFileInitialMessage.filesize());
  ASSERT_EQ(fileInitialMessage.chunksize(),
            recoveredFileInitialMessage.chunksize());
  ASSERT_EQ(fileInitialMessage.lastfile(),
            recoveredFileInitialMessage.lastfile());
  ASSERT_TRUE(google::protobuf::util
              ::MessageDifferencer::Equals(fileInitialMessage,
                                         recoveredFileInitialMessage));
}

TEST(MessagingTest, ChunkHeaderTest)
{
  autocomp::messaging::ChunkHeader chunkHeader, recoveredChunkHeader;
  chunkHeader.set_compressor(autocomp::LZMA);
  chunkHeader.set_chunkposition(1234);

  std::vector<char> buffer;

  ASSERT_TRUE(autocomp::serializeMessage(chunkHeader, buffer));
  ASSERT_TRUE(autocomp::deserializeMessage(buffer, recoveredChunkHeader));

  ASSERT_EQ(chunkHeader.compressor(),
            recoveredChunkHeader.compressor());
  ASSERT_EQ(chunkHeader.chunkposition(),
            recoveredChunkHeader.chunkposition());
  ASSERT_TRUE(google::protobuf::util
              ::MessageDifferencer::Equals(chunkHeader, recoveredChunkHeader));
}

TEST(MessagingTest, ErrorMessageTest)
{
  autocomp::messaging::ErrorMessage errorMessage, recoveredErrorMessage;
  errorMessage.set_message("This is the message");

  std::vector<char> buffer;

  ASSERT_TRUE(autocomp::serializeMessage(errorMessage, buffer));
  ASSERT_TRUE(autocomp::deserializeMessage(buffer, recoveredErrorMessage));

  ASSERT_EQ(errorMessage.message(),
            recoveredErrorMessage.message());
  ASSERT_TRUE(google::protobuf::util
              ::MessageDifferencer::Equals(errorMessage,
                                           recoveredErrorMessage));
}

TEST(MessagingTest, InitializationTest)
{
  autocomp::messaging::FileInitialMessage fileInitialMessage;
  ASSERT_FALSE(fileInitialMessage.IsInitialized());
  fileInitialMessage.set_filename("filename");
  fileInitialMessage.set_filesize(1);
  fileInitialMessage.set_chunksize(1);
  ASSERT_TRUE(fileInitialMessage.IsInitialized());
}

#endif //AC_MESSAGING_TEST_HPP