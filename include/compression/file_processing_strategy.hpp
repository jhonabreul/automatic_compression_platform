/**
 *  AutoComp File Processing Strategy
 *  file_processing_strategy.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/30/2018
 */

#ifndef AC_FILE_PROCESSING_STRATEGY_HPP
#define AC_FILE_PROCESSING_STRATEGY_HPP

#include <string>
#include <memory>
#include <fstream>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "io/directory_explorer.hpp"

namespace autocomp {

/** 
 * Abstract class that will serve as an interface for File Processors
 */
class FileProcessingStrategy
{
protected:

  /**
   * Working path.
   * 
   * This is useful when a whole directory is being processed.
   */
  std::string workingPath;

  /**
   * The name of the current file being processed. 
   */
  std::string currentFileName;

  /**
   * Input stream for the current file being processed. 
   */
  std::ifstream source;

  /**
   * Current file size
   */
  size_t currentFileSize;

  /**
   * Current file consumed bytes
   */
  size_t currentFileReadBytes;

  /**
   * Each file is processed in chunks of size chunkSize.
   */
  unsigned int chunkSize;

  /**
   * Chunk size in bytes.
   */
  size_t chunkSizeBytes;

  /**
   * Explorer used to traverse a directory.
   */
  std::unique_ptr<DirectoryExplorer> directoryExplorer;

public:

  FileProcessingStrategy(const unsigned int & chunkSize);

  /**
   * Gets current chunk size
   *
   * @returns The current chunk size
   */
  int getChunkSize();

  /**
   * Sets the chunk size that will be used for file processing.
   *
   * @param chunkSize Chunk size in KB
   */
  void setChunkSize(const unsigned int & chunkSize);

  /**
   * Prepares any input streams for processing the given file or directory
   *
   * @param path Path if the file or directory to be processed
   *
   * @throws exceptions::IOError If an I/O error occurs
   * @throws std::bad_alloc If a memory allocation error occurs
   */
  void preparePath(const std::string & path);

  /**
   * Verifies if there is another file to be processed in the working path
   *
   * @returns true if there is another file to be processed in the path
   */
  bool hasNextFile() const;

  /**
   * Opens and prepares the next file.
   *
   * @throws exceptions::IOError If an I/O error occurs
   */
  virtual size_t openNextFile() = 0;

  /**
   * Verifies if there is chunk to read from the current file being processed
   *
   * @returns true if there is another chunk to read from the current file
   */
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
  virtual Compressor getNextChunk(Buffer & chunk) = 0;

  /**
   * Gets the name of the current file being processed.
   *
   * @returns The name of the current file being processed.
   */
  std::string getCurrentFileName() const;

  /**
   * Gets the size of the current file being processed.
   *
   * @returns The size of the current file being processed.
   */
  size_t getCurrentFileSize() const;

protected:

  /**
   * Calculates the current file size.
   */
  void calculateFileSize();

}; // class FileProcessingStrategy

} // namespace autocomp

#endif // AC_FILE_PROCESSING_STRATEGY_HPP