/**
 *  AutoComp Pre-Compressing File Processor
 *  pre_compressing_file_processor.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/31/2018
 */

#include "compression/pre_compressing_file_processor.hpp"

namespace autocomp {

// Instantiates a file processor. The files are going to be processed in
// chunks of size chunkSize
PreCompressingFileProcessor::PreCompressingFileProcessor()
 : FileProcessingStrategy(1024),
   leveledCompressors{
      {ZLIB,  std::make_shared<ZlibCompressor>()},
      {LZO,   std::make_shared<LZOCompressor>()},
      {BZIP2, std::make_shared<Bzip2Compressor>()},
      {LZMA,  std::make_shared<LZMACompressor>()},
      {FPC,   std::make_shared<FPCCompressor>()}
   },
   fileExtenssions{
      {ZLIB,    ".gz"},
      {SNAPPY,  ".snappy"},
      {LZO,     ".lzop"},
      {BZIP2,   ".bz2"},
      {LZMA,    ".lzma"},
      {FPC,     ".fpc"},
      {COPY,    ""}
   },
   compressorScripts{
      {ZLIB,    constants::ZLIB_SCRIPT},
      //{SNAPPY,  constants::SNAPPY_SCRIPT},
      {LZO,     constants::LZO_SCRIPT},
      {BZIP2,   constants::BZIP2_SCRIPT},
      {LZMA,    constants::LZMA_SCRIPT},
      {FPC,     constants::FPC_SCRIPT}
   },
   compressedFileSize(0),
   compressor(COPY),
   compressionLevel(-1)
{}

// Opens and prepares the next file
size_t PreCompressingFileProcessor::openNextFile()
{
  if (not this->hasNextFile()) {
    return 0;
  }

  if (this->source.is_open()) {
    this->source.close();
  }

  this->currentFileName = this->directoryExplorer->getNextFileName();

  // Compress file first and set currentFileName to the compressedName
  this->currentCompressedFileName = this->currentFileName +
                                    this->fileExtenssions.at(this->compressor);
  this->compressFile(this->currentFileName, this->currentCompressedFileName,
                     this->compressionLevel);

  // Open compressed file and calculate its size
  this->source.open(this->currentCompressedFileName,
                    std::ifstream::in | std::ifstream::binary);

  if (this->source.fail()) {
    throw exceptions::IOError(std::string("Could not open current file: ")
                                  .append(this->currentCompressedFileName));
  }

  FileProcessingStrategy::calculateFileSize();
  this->compressedFileSize = this->currentFileSize;

  // Calculate original file size
  this->currentFileSize = this->calculateFileSize(this->currentFileName);
  this->currentFileReadBytes = 0;

  return this->currentFileSize;
}

bool PreCompressingFileProcessor::hasNextChunk() const
{
  return not this->source.eof() and not this->source.fail() and
         this->currentFileReadBytes < this->compressedFileSize;
}

// Processes the next available file chunk and return the compressor used
// for processing it. In case of no compression at all, COPY is returned. If
// the processore chooses to compress and a compression error occurs, of if
// any other kind of error occurs (like I/O), no compression is done, the
// whole original chunk is copied to the buffer and COPY is returned.
Compressor PreCompressingFileProcessor::getNextChunk(Buffer & chunk)
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

  if (chunk.getCapacity() < this->chunkSizeBytes) {
    chunk.resize(this->chunkSizeBytes);
  }

  this->source.read(chunk.getData(), this->chunkSizeBytes);
  this->currentFileReadBytes += this->source.gcount();
  chunk.setSize(this->source.gcount());

  // Remove compressed file
  if (not this->hasNextChunk()) {
    this->source.close();

    if (this->compressor != COPY) {
      ::remove(this->currentCompressedFileName.c_str());
    }
  }

  return this->compressor;
}

// Sets Compressor to use for file processing
void PreCompressingFileProcessor::setCompressor(const Compressor & compressor,
                                                const int & compressionLevel)
{
  if (not Compressor_IsValid(compressor)) {
    throw exceptions::InvalidCompressorError(compressor,
                                             "Setting and invalid compressor "
                                             "in PreCompressingFileProcessor");
  }

  if (compressor != SNAPPY and compressor != COPY) {
    auto leveledCompressor = this->leveledCompressors.at(compressor);

    if (compressionLevel == -1) {
      this->compressionLevel = leveledCompressor->getDefaultCompressionLevel();
    }
    else if (compressionLevel >= leveledCompressor->getMinCompressionLevel() or 
             compressionLevel <= leveledCompressor->getMaxCompressionLevel()) {
      this->compressionLevel = compressionLevel;
    }
    else {
      throw exceptions::InvalidCompressionLevelError(
          Compressor_Name(compressor), compressionLevel, ""
        );
    }
  }

  this->compressor = compressor;
}

size_t
PreCompressingFileProcessor::calculateFileSize(const std::string & filename)
{
  std::ifstream file(filename, std::ifstream::in | std::ifstream::binary);

  if (file.fail()) {
    throw exceptions::IOError(std::string("Could not open file: ")
                                  .append(filename));
  }

  file.seekg(0, std::ios_base::end);

  return file.tellg();
}

void PreCompressingFileProcessor::compressFile(const std::string & inFilename,
                                               const std::string & outFilename,
                                               const int & compressionLevel)
{
  int errnoValue = 0;

  if (this->compressor == COPY) {
    return;
  }

  std::string compressionScript = this->compressorScripts.at(this->compressor);

  pid_t pid = ::vfork();

  if (pid == -1) {
    throw exceptions::CompressionError(Compressor_Name(this->compressor), 0, 0,
                                       "Could not fork child (compression) "
                                       "process");
  }

  // Child
  if (pid == 0) {
    execlp(compressionScript.c_str(), compressionScript.c_str(),
           inFilename.c_str(), outFilename.c_str(),
           std::to_string(compressionLevel).c_str(), nullptr);

    errnoValue = errno;

    _exit(1);
  }

  // Parent
  // The child exited or exec'ed:
  if (errnoValue) {
    std::string errorMessage("Could not execute compression script ");
    errorMessage.append(compressionScript)
                .append(" because child (compression) process exited with"
                        " errno code ")
                .append(std::to_string(errnoValue))
                .append(" (")
                .append(std::strerror(errnoValue))
                .append(")");

    throw exceptions::CompressionError(Compressor_Name(this->compressor), 0, 0,
                                       errorMessage);
  }

  int childExitStatus;

  if (::waitpid(pid, &childExitStatus, 0) == -1) {
    throw exceptions::CompressionError(Compressor_Name(this->compressor), 0, 0,
                                       "waitpid() exited with error (-1) when "
                                       "waiting for child (compression) "
                                       "process");
  }

  if (not WIFEXITED(childExitStatus)) {
    throw exceptions::CompressionError(Compressor_Name(this->compressor), 0, 0,
                                       "Child (compression) process did not "
                                       "exited successfully");
  }

  if (WEXITSTATUS(childExitStatus)) {
    std::string errorMessage("Child (compressor) process exited with status ");
    errorMessage.append(std::to_string(WEXITSTATUS(childExitStatus)));

    throw exceptions::CompressionError(Compressor_Name(this->compressor), 0, 0,
                                       errorMessage);
  }

  // Compression done
}

void
PreCompressingFileProcessor::decompressFile(const std::string & inFilename,
                                            const std::string & outFilename,
                                            Compressor & compressor)
{
  if (not Compressor_IsValid(compressor)) {
    throw exceptions::InvalidCompressorError(compressor,
                                             "Compressor not found");
  }

  if (compressor == COPY) {
    return;
  }

  int errnoValue = 0;

  std::string compressionScript = this->compressorScripts.at(compressor);

  pid_t pid = ::vfork();

  if (pid == -1) {
    throw exceptions::DecompressionError(Compressor_Name(compressor), 0, 0,
                                         "Could not fork child (decompression) "
                                         "process");
  }

  // Child
  if (pid == 0) {
    execlp(compressionScript.c_str(), compressionScript.c_str(),
           "-d", inFilename.c_str(), outFilename.c_str(), nullptr);

    errnoValue = errno;

    _exit(1);
  }

  // Parent
  // The child exited or exec'ed:
  if (errnoValue) {
    std::string errorMessage("Could not execute decompression script ");
    errorMessage.append(compressionScript)
                .append(" because child (decompression) process exited with"
                        " errno code ")
                .append(std::to_string(errnoValue))
                .append(" (")
                .append(std::strerror(errnoValue))
                .append(")");

    throw exceptions::DecompressionError(Compressor_Name(compressor), 0, 0,
                                         errorMessage);
  }

  int childExitStatus;

  if (::waitpid(pid, &childExitStatus, 0) == -1) {
    throw exceptions::DecompressionError(Compressor_Name(compressor),
                                         0, 0,
                                         "waitpid() exited with error (-1) when"
                                         " waiting for child (decompression)"
                                         " process");
  }

  if (not WIFEXITED(childExitStatus)) {
    throw exceptions::DecompressionError(Compressor_Name(compressor),
                                         0, 0,
                                         "Child (decompression) process did not"
                                         " exited successfully");
  }

  if (WEXITSTATUS(childExitStatus)) {
    std::string errorMessage("Child (decompression) process exited with "
                             "status ");
    errorMessage.append(std::to_string(WEXITSTATUS(childExitStatus)));

    throw exceptions::DecompressionError(Compressor_Name(compressor), 0, 0,
                                       errorMessage);
  }
}

} // namespace autocomp