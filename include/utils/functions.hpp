/**
 *  AutoComp Common Functions
 *  functions.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/26/2018
 */

#ifndef AC_COMMON_FUNCTIONS_HPP
#define AC_COMMON_FUNCTIONS_HPP

#include <unistd.h>
#include <array>
#include <strings.h>

namespace autocomp
{

inline int bytecounting(const unsigned char * data,
                        const std::size_t & dataSize)
{
  const int nBytes = 256;
  static std::array<std::size_t, nBytes> byteOccurrenceContainer;
  std::size_t * byteOccurrences = byteOccurrenceContainer.data();
  ::bzero(byteOccurrences, nBytes * sizeof(std::size_t));

  for (int i = 0; i < dataSize; i++) {
    byteOccurrences[data[i]]++;
  }

  std::size_t threshold = dataSize / nBytes;
  int bytecount = 0;

  for (int i = 0; i < nBytes; i++) {
    if (byteOccurrences[i] >= threshold) {
      bytecount++;
    }
  }

  return bytecount;
}

inline int bytecounting(const char * data, const std::size_t & dataSize)
{
  return bytecounting(reinterpret_cast<const unsigned char *>(data), dataSize);
}

} // namespace autocomp

#endif // AC_COMMON_FUNCTIONS_HPP