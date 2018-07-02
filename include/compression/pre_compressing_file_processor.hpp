/**
 *  AutoComp Pre-Compressing File Processor
 *  pre_compressing_file_processor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/31/2018
 */

#ifndef AC_PRE_COMPRESSING_FILE_PROCESSOR_HPP
#define AC_PRE_COMPRESSING_FILE_PROCESSOR_HPP

#include <string>
#include <memory>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio> // remove

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "utils/constants.hpp"
#include "messaging/compressor.pb.h"
#include "io/directory_explorer.hpp"
#include "compression/leveled_compressor.hpp"
#include "compression/automatic_compression_strategy.hpp"
#include "compression/file_processing_strategy.hpp"
#include "compression/zlib_compressor.hpp"
#include "compression/snappy_compressor.hpp"
#include "compression/lzo_compressor.hpp"
#include "compression/bzip2_compressor.hpp"
#include "compression/lzma_compressor.hpp"
#include "compression/fpc_compressor.hpp"

namespace autocomp {

/** 
 * This class is intended to be used for file preparation for transmission.
 *
 * This preparation is usually compression before transmission but at file level
 */
class PreCompressingFileProcessor : public FileProcessingStrategy
{

  /**
   * The compressor to be used
   */
  Compressor compressor;

  unsigned int compressionLevel;

  const std::map<Compressor,
                 std::shared_ptr<LeveledCompressor>> leveledCompressors;

  const std::map<Compressor, std::string> fileExtenssions;

  const std::map<Compressor, std::string> compressorScripts;

  size_t compressedFileSize;

  std::string currentCompressedFileName;

public:

  /**
   * Instantiates a file processor. The files are going to be processed in
   * chunks of size chunkSize.
   *
   * @throws exceptions::IOError If an I/O error occurs 
   */
  PreCompressingFileProcessor();

  /**
   * Opens and prepares the next file.
   *
   * @throws exceptions::IOError If an I/O error occurs
   */
  size_t openNextFile();

  bool hasNextChunk() const;

  /**
   * Processes the next available file chunk and return the compressor used
   * for processing it. In case of no compression at all, COPY is returned. If
   * the processore chooses to compress and a compression error occurs, of if
   * any other kind of error occurs (like I/O), no compression is done, the
   * whole original chunk is copied to the buffer and COPY is returned.
   *
   * @param chunk The buffer where the processed chunk is going to be stored.
   *
   * @returns The compressor used for processing the chunk. In the case of
   *          no compression at all, COPY is returned.
   */
  Compressor getNextChunk(Buffer & chunk);

  /**
   * Gets the size of the current file being processed.
   *
   * @returns The size of the current file being processed.
   */
  size_t getCurrentFileSize() const;

  /**
   * Sets Compressor to use for file processing
   *
   * @param compressor Compressor to use for file processing
   */
  void setCompressor(const Compressor & compressor,
                     const int & compressionLevel = -1);

private:

  size_t calculateFileSize(const std::string & filename);

  void compressFile(const std::string & inFilename,
                    const std::string & outFilename,
                    const int & compressionLevel = -1);

public:

  void decompressFile(const std::string & inFilename,
                      const std::string & outFilename,
                      Compressor & compressor);

}; // class PreCompressingFileProcessor

} // namespace autocomp

#endif // AC_PRE_COMPRESSING_FILE_PROCESSOR_HPP