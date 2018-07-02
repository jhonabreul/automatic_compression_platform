/**
 *  AutoComp Protobuf helper functions
 *  protobuf_utils.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/01/2018
*/

#ifndef AC_PROTOBUF_UTILS_HPP
#define AC_PROTOBUF_UTILS_HPP

#include <utility>
#include <vector>

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "utils/buffer.hpp"

namespace autocomp
{

template<typename T>
inline bool serializeMessage(T & message, std::vector<char> & buffer)
{
  size_t messageLength = message.ByteSizeLong();

  buffer = std::vector<char>(messageLength);

  google::protobuf::io::ArrayOutputStream arrayOutput(
      reinterpret_cast<google::protobuf::uint8 *>(buffer.data()), buffer.size()
    );
  google::protobuf::io::CodedOutputStream codedOutput(&arrayOutput);

  return message.SerializeToCodedStream(&codedOutput);
}

template<typename T>
inline bool serializeMessage(T & message, Buffer & buffer)
{
  size_t messageLength = message.ByteSizeLong();

  if (buffer.getCapacity() < messageLength) {
    buffer.resize(messageLength);
  }

  google::protobuf::io::ArrayOutputStream arrayOutput(
      reinterpret_cast<google::protobuf::uint8 *>(buffer.getData()),
      buffer.getCapacity()
    );
  google::protobuf::io::CodedOutputStream codedOutput(&arrayOutput);

  bool serializationResult = message.SerializeToCodedStream(&codedOutput);

  if (serializationResult) {
    buffer.setSize(messageLength);
  }

  return serializationResult;
}

template<typename T>
inline bool deserializeMessage(std::vector<char> & buffer, T & message)
{
  google::protobuf::io::ArrayInputStream arrayInput(
      reinterpret_cast<google::protobuf::uint8 *>(buffer.data()), buffer.size()
    );
  google::protobuf::io::CodedInputStream codedInput(&arrayInput);

  return message.ParseFromCodedStream(&codedInput);
}

} // namespace autocomp

#endif // AC_PROTOBUF_UTILS_HPP