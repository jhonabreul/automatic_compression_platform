/**
 *  AutoComp File Processor
 *  file_processor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/20/2018
 */

#ifndef AC_FILE_PROCESSOR_HPP
#define AC_FILE_PROCESSOR_HPP

#include <string>
#include <memory>
#include <fstream>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "io/directory_explorer.hpp"
#include "compression/automatic_compression_strategy.hpp"
#include "compression/file_processing_strategy.hpp"

namespace autocomp {

/** 
 * This class is intended to be used for file preparation for transmission.
 *
 * This preparation is usually compression before transmission.
 */
class FileProcessor : public FileProcessingStrategy
{

  /**
   * The compressor to be used
   */
  std::shared_ptr<AutomaticCompressionStrategy> compressor;

public:

  /**
   * Instantiates a file processor. The files are going to be processed in
   * chunks of size chunkSize.
   *
   * @param chunkSize Chunk size in KB
   * @param compressor Compressor to use for file processing
   *
   * @throws exceptions::IOError If an I/O error occurs 
   */
  FileProcessor(const unsigned int & chunkSize,
                const std::shared_ptr<AutomaticCompressionStrategy> compressor);

  /**
   * Opens and prepares the next file.
   *
   * @throws exceptions::IOError If an I/O error occurs
   */
  size_t openNextFile();

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
  Compressor getNextChunk(Buffer & chunk,
                          const std::shared_ptr<AutomaticCompressionStrategy>
                            compressor);

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
  void setCompressor(const std::shared_ptr<AutomaticCompressionStrategy>
                        compressor);

}; // class FileProcessor

} // namespace autocomp

#endif // AC_FILE_PROCESSOR_HPP