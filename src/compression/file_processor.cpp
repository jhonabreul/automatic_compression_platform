/**
 *  AutoComp File Processor
 *  file_processor.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/20/2018
 */

#include "compression/file_processor.hpp"

namespace autocomp {

// Instantiates a file processor. The files are going to be processed in
// chunks of size chunkSize
FileProcessor::FileProcessor(const unsigned int & chunkSize,
                             const std::shared_ptr<AutomaticCompressionStrategy>
                                compressor)
 : FileProcessingStrategy(chunkSize),
   compressor(compressor)
{}

// Opens and prepares the next file
size_t FileProcessor::openNextFile()
{
  if (not this->hasNextFile()) {
    return 0;
  }

  if (this->source.is_open()) {
    this->source.close();
  }

  this->currentFileName = this->directoryExplorer->getNextFileName();
  this->source.open(this->currentFileName,
                    std::ifstream::in | std::ifstream::binary);

  if (this->source.fail()) {
    throw exceptions::IOError(std::string("Could not open current file: ")
                                .append(this->currentFileName));
  }

  this->calculateFileSize();
  this->currentFileReadBytes = 0;

  return this->currentFileSize;
}

// Processes the next available file chunk and return the compressor used
// for processing it. In case of no compression at all, COPY is returned. If
// the processore chooses to compress and a compression error occurs, of if
// any other kind of error occurs (like I/O), no compression is done, the
// whole original chunk is copied to the buffer and COPY is returned.
Compressor FileProcessor::getNextChunk(Buffer & chunk)
{
  if (not this->source.is_open()) {
    throw exceptions::IOError(std::string("There is no open input stream for "
                                          "the current file ")
                                .append(this->currentFileName));
  }

  if (not this->hasNextChunk()) {
    throw exceptions::IOError(std::string("There is no more data to read from "
                                          "the input stream corresponding to "
                                          "the current file ")
                                .append(this->currentFileName));
  }

  if (chunk.getCapacity() < 1.1 * this->chunkSizeBytes) {
    chunk.resize(1.1 * this->chunkSizeBytes);
  }

  Buffer inData(this->chunkSizeBytes);
  this->source.read(inData.getData(), this->chunkSizeBytes);
  this->currentFileReadBytes += this->source.gcount();
  inData.setSize(this->source.gcount());

  Compressor usedCompressor;

  try {
    usedCompressor = this->compressor->compress(inData, chunk);
  }
  catch (exceptions::CompressionError & error) {
    usedCompressor = COPY;
  }

  if (usedCompressor == COPY) {
    chunk.swap(inData);
  }

  return usedCompressor;
}

// Processes the next available file chunk and return the compressor used
// for processing it. In case of no compression at all, COPY is returned. If
// the processore chooses to compress and a compression error occurs, of if
// any other kind of error occurs (like I/O), no compression is done, the
// whole original chunk is copied to the buffer and COPY is returned.
Compressor FileProcessor::getNextChunk(
    Buffer & chunk,
    const std::shared_ptr<AutomaticCompressionStrategy> compressor
  )
{
  auto oldCompressor = this->compressor;
  this->setCompressor(compressor);
  Compressor usedCompressor;

  try {
    usedCompressor = this->getNextChunk(chunk);
  }
  catch (exceptions::IOError & error) {
    this->compressor = oldCompressor;
    throw error;
  }

  this->compressor = oldCompressor;

  return usedCompressor;
}

// Sets Compressor to use for file processing
void FileProcessor::setCompressor(
    const std::shared_ptr<AutomaticCompressionStrategy> compressor
  )
{
  if (compressor != nullptr) {
    this->compressor = compressor;
  }
}

} // namespace autocomp