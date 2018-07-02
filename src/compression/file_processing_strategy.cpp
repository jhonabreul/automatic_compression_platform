/**
 *  AutoComp File Processor
 *  file_processor.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/20/2018
 */

#include "compression/file_processing_strategy.hpp"

namespace autocomp {

FileProcessingStrategy::FileProcessingStrategy(const unsigned int & chunkSize)
  : chunkSize(chunkSize),
    chunkSizeBytes(chunkSize * 1024),
    currentFileSize(0),
    currentFileReadBytes(0),
    currentFileName("")
{}

// Gets current chunk size
int FileProcessingStrategy::getChunkSize()
{
  return this->chunkSize;
}

// Sets the chunk size that will be used for file processing.
void FileProcessingStrategy::setChunkSize(const unsigned int & chunkSize)
{
  this->chunkSize = chunkSize;
  this->chunkSizeBytes = chunkSize * 1024;
}

// Prepares any input streams for processing the given file or directory
void FileProcessingStrategy::preparePath(const std::string & path)
{
  this->workingPath = path;
  this->directoryExplorer.reset(new DirectoryExplorer(this->workingPath));
}

// Verifies if there is another file to be processed in the working path
bool FileProcessingStrategy::hasNextFile() const
{
  return this->directoryExplorer and this->directoryExplorer->hasNextFile();
}

// Verifies if there is chunk to read from the current file being processed
bool FileProcessingStrategy::hasNextChunk() const
{
  return not this->source.eof() and not this->source.fail() and
         this->currentFileReadBytes < this->currentFileSize;
}

// Gets the name of the current file being processed.
std::string FileProcessingStrategy::getCurrentFileName() const
{
  return this->currentFileName;
}

// Gets the size of the current file being processed
size_t FileProcessingStrategy::getCurrentFileSize() const
{
  return this->currentFileSize;
}

// Calculates the current file size
void FileProcessingStrategy::calculateFileSize()
{
  this->source.seekg(0, std::ios_base::end);
  this->currentFileSize = this->source.tellg();
  this->source.seekg(0, std::ios_base::beg);
}

} // namespace autocomp