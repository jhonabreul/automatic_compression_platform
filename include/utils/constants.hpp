/**
 *  AutoComp Constants
 *  constants.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/17/2018
*/

#ifndef AC_CONSTANTS_HPP
#define AC_CONSTANTS_HPP

#include <string>
#include <map>

#include "messaging/compressor.pb.h"

namespace autocomp
{
  namespace constants
  {
    const std::string LOG_DIR("./log");

    const std::string LOG_FILE_PREFIX("AutoComp");

    const std::string LOG_FILE_ID("events");

    const std::string CSV_LOG_FILE_EXTENSSION(".csv");

    const std::string PERFORMANCE_LOG_FILE_NAME_PREFIX("AutoComp.Compressors"
                                                       "Performance.");

    const std::string SHUTDOWN_PIPE_NAME("/tmp/autocomp.fifo");

    const unsigned short DEFAULT_SERVER_PORT = 25111;

    // Script paths

    const std::string ZLIB_SCRIPT("./scripts/zlib.sh");
    const std::string SNAPPY_SCRIPT("./scripts/snappy.sh");
    const std::string LZO_SCRIPT("./scripts/lzo.sh");
    const std::string BZIP2_SCRIPT("./scripts/bzip2.sh");
    const std::string LZMA_SCRIPT("./scripts/lzma.sh");
    const std::string FPC_SCRIPT("./scripts/fpc.sh");
    const std::string CPU_MODULATOR_SCRIPT("./src/tools/resource_modulators/"
                                           "cpu_stressor.py");
    const std::string BANDWIDTH_MODULATOR_SCRIPT("./src/tools/"
                                                 "resource_modulators/"
                                                 "bandwidth_limiter.py");

    const std::map<Compressor, std::string> COMPRESSED_FILE_EXTENSSIONS{
                                                {ZLIB,    ".gz"},
                                                {SNAPPY,  ".snappy"},
                                                {LZO,     ".lzop"},
                                                {BZIP2,   ".bz2"},
                                                {LZMA,    ".lzma"},
                                                {FPC,     ".fpc"},
                                                {COPY,    ""}
                                              };

    const std::size_t MAX_STRING_LENGTH = 1000;

    const std::string DECISION_TREE_FILENAME("./models/decision_tree.txt");

  } // namespace constants
} // namespace autocomp

#endif // AC_CONSTANTS_HPP