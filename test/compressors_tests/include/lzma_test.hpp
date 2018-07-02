#ifndef LZMA_TEST_H
#define LZMA_TEST_H

/* C++ System Headers */
#include <string>
#include <memory>
#include <cmath>

/* External headers */
#include "gtest/gtest.h"

extern "C" {
  #include "lzma.h"
}

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"

class LZMATest : public ::testing::Test
{
protected:

  std::string originalData;
  unsigned char * originalBuffer;
  unsigned char * compressedBuffer;
  unsigned char * uncompressedBuffer;
  int compressedCapacity, uncompressedCapacity;
  int compressedSize, uncompressedSize;

  const int bufferSize = 4096;

  void SetUp()
  {
    // Read file contents
    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::compressionTestFilename
        );
    });

    compressedCapacity = 2 * originalData.size();
    uncompressedCapacity = originalData.size();

    ASSERT_NO_THROW({
      originalBuffer = new unsigned char[originalData.size()];
      compressedBuffer = new unsigned char[compressedCapacity];
      uncompressedBuffer = new unsigned char[uncompressedCapacity];
    });

    memcpy(originalBuffer, originalData.data(), originalData.size());
  }
     
  void TearDown()
  {
    delete[] originalBuffer;
    delete[] compressedBuffer;
    delete[] uncompressedBuffer;
  }

  bool initEncoder(lzma_stream * stream, uint32_t compressionLevel)
  {
    memset(stream, 0, sizeof(*stream));

    // Initialize the encoder using a preset. Set the integrity to check
    // to CRC64, which is the default in the xz command line tool. If
    // the .xz file needs to be decompressed with XZ Embedded, use
    // LZMA_CHECK_CRC32 instead.
    lzma_ret initResult = lzma_easy_encoder(stream, compressionLevel,
                                            LZMA_CHECK_CRC64);

    // Return successfully if the initialization went fine.
    if (initResult == LZMA_OK) {
      return true;
    }

    // Something went wrong. The possible errors are documented in
    // lzma/container.h (src/liblzma/api/lzma/container.h in the source
    // package or e.g. /usr/include/lzma/container.h depending on the
    // install prefix).
    std::string message;

    switch (initResult) {
      case LZMA_MEM_ERROR:
        message = "Memory allocation failed";
        break;

      case LZMA_OPTIONS_ERROR:
        message = "Specified preset is not supported";
        break;

      case LZMA_UNSUPPORTED_CHECK:
        message = "Specified integrity check is not supported";
        break;

      default:
        // This is most likely LZMA_PROG_ERROR indicating a bug in
        // this program or in liblzma. It is inconvenient to have a
        // separate error message for errors that should be impossible
        // to occur, but knowing the error code is important for
        // debugging. That's why it is good to print the error code
        // at least when there is no good error message to show.
        message = "Unknown error, possibly a bug";
        break;
    }

    std::cerr << "COMPRESSOR INIT ERROR: " << message << std::endl;

    return false;
  }

  bool compress(const unsigned char * inBuffer, const int inBufferSize,
                unsigned char * outBuffer, int & outBufferSize,
                int compressionLevel)
  {
    std::shared_ptr<lzma_stream> stream = std::make_shared<lzma_stream>();

    if (!initEncoder(stream.get(), compressionLevel)) {
      return false;
    }

    bool retVal = false;

    // This will be LZMA_RUN until the end of the input file is reached.
    // This tells lzma_code() when there will be no more input.
    lzma_action action = LZMA_RUN;

    // Initialize the input and output pointers. Initializing next_in
    // and avail_in isn't really necessary when we are going to encode
    // just one file since LZMA_STREAM_INIT takes care of initializing
    // those already. But it doesn't hurt much and it will be needed
    // if encoding more than one file like we will in 02_decompress.c.
    //
    // While we don't care about strm->total_in or strm->total_out in this
    // example, it is worth noting that initializing the encoder will
    // always reset total_in and total_out to zero. But the encoder
    // initialization doesn't touch next_in, avail_in, next_out, or
    // avail_out.
    stream->next_in = nullptr;
    stream->avail_in = 0;
    stream->next_out = outBuffer;
    stream->avail_out = outBufferSize;

    int consumed = 0;

    // Loop until the file has been successfully compressed or until
    // an error occurs.
    while (true) {
      // Fill the input buffer if it is empty.
      if (stream->avail_in == 0) {
        if (consumed < inBufferSize) {
          if (inBufferSize - consumed > bufferSize) {
            stream->next_in = inBuffer + consumed;
            stream->avail_in = bufferSize;
          } else {
            stream->avail_in = inBufferSize - consumed;
          }

          consumed += stream->avail_in;
        }
        else {
          action = LZMA_FINISH;
        }          
      }

      // Tell liblzma do the actual encoding.
      //
      // This reads up to strm->avail_in bytes of input starting
      // from strm->next_in. avail_in will be decremented and
      // next_in incremented by an equal amount to match the
      // number of input bytes consumed.
      //
      // Up to strm->avail_out bytes of compressed output will be
      // written starting from strm->next_out. avail_out and next_out
      // will be incremented by an equal amount to match the number
      // of output bytes written.
      //
      // The encoder has to do internal buffering, which means that
      // it may take quite a bit of input before the same data is
      // available in compressed form in the output buffer.
      lzma_ret compressionResult = lzma_code(stream.get(), action);

      // If the output buffer is full or if the compression finished
      // successfully, write the data from the output bufffer to
      // the output file.
      if (stream->avail_out == 0 and stream->avail_in > 0) {
        retVal = false;

        break;
      }

      // Normally the return value of lzma_code() will be LZMA_OK
      // until everything has been encoded.
      if (compressionResult != LZMA_OK) {
        // Once everything has been encoded successfully, the
        // return value of lzma_code() will be LZMA_STREAM_END.
        //
        // It is important to check for LZMA_STREAM_END. Do not
        // assume that getting ret != LZMA_OK would mean that
        // everything has gone well.
        if (compressionResult == LZMA_STREAM_END) {
          outBufferSize -= stream->avail_out;
          retVal = true;

          break;
        }

        // It's not LZMA_OK nor LZMA_STREAM_END,
        // so it must be an error code. See lzma/base.h
        // (src/liblzma/api/lzma/base.h in the source package
        // or e.g. /usr/include/lzma/base.h depending on the
        // install prefix) for the list and documentation of
        // possible values. Most values listen in lzma_ret
        // enumeration aren't possible in this example.
        std::string message;

        switch (compressionResult) {
        case LZMA_MEM_ERROR:
          message = "Memory allocation failed";
          break;

        case LZMA_DATA_ERROR:
          // This error is returned if the compressed
          // or uncompressed size get near 8 EiB
          // (2^63 bytes) because that's where the .xz
          // file format size limits currently are.
          // That is, the possibility of this error
          // is mostly theoretical unless you are doing
          // something very unusual.
          //
          // Note that strm->total_in and strm->total_out
          // have nothing to do with this error. Changing
          // those variables won't increase or decrease
          // the chance of getting this error.
          message = "File size limits exceeded";
          break;

        default:
          // This is most likely LZMA_PROG_ERROR, but
          // if this program is buggy (or liblzma has
          // a bug), it may be e.g. LZMA_BUF_ERROR or
          // LZMA_OPTIONS_ERROR too.
          //
          // It is inconvenient to have a separate
          // error message for errors that should be
          // impossible to occur, but knowing the error
          // code is important for debugging. That's why
          // it is good to print the error code at least
          // when there is no good error message to show.
          message = "Unknown error, possibly a bug";
          break;
        }

        std::cerr << "COMPRESSION ERROR: " << message
                  << " (error code " << compressionResult << ")"
                  << std::endl;

        break;
      }
    }

    lzma_end(stream.get());

    return retVal;
  }
  
  bool initDecoder(lzma_stream * stream)
  {
    memset(stream, 0, sizeof(*stream));

    // Initialize a .xz decoder. The decoder supports a memory usage limit
    // and a set of flags.
    //
    // The memory usage of the decompressor depends on the settings used
    // to compress a .xz file. It can vary from less than a megabyte to
    // a few gigabytes, but in practice (at least for now) it rarely
    // exceeds 65 MiB because that's how much memory is required to
    // decompress files created with "xz -9". Settings requiring more
    // memory take extra effort to use and don't (at least for now)
    // provide significantly better compression in most cases.
    //
    // Memory usage limit is useful if it is important that the
    // decompressor won't consume gigabytes of memory. The need
    // for limiting depends on the application. In this example,
    // no memory usage limiting is used. This is done by setting
    // the limit to UINT64_MAX.
    //
    // The .xz format allows concatenating compressed files as is:
    //
    //     echo foo | xz > foobar.xz
    //     echo bar | xz >> foobar.xz
    //
    // When decompressing normal standalone .xz files, LZMA_CONCATENATED
    // should always be used to support decompression of concatenated
    // .xz files. If LZMA_CONCATENATED isn't used, the decoder will stop
    // after the first .xz stream. This can be useful when .xz data has
    // been embedded inside another file format.
    //
    // Flags other than LZMA_CONCATENATED are supported too, and can
    // be combined with bitwise-or. See lzma/container.h
    // (src/liblzma/api/lzma/container.h in the source package or e.g.
    // /usr/include/lzma/container.h depending on the install prefix)
    // for details.
    lzma_ret initResult = lzma_stream_decoder(stream, UINT64_MAX,
                                              LZMA_CONCATENATED);

    // Return successfully if the initialization went fine.
    if (initResult == LZMA_OK) {
      return true;
    }

    // Something went wrong. The possible errors are documented in
    // lzma/container.h (src/liblzma/api/lzma/container.h in the source
    // package or e.g. /usr/include/lzma/container.h depending on the
    // install prefix).
    //
    // Note that LZMA_MEMLIMIT_ERROR is never possible here. If you
    // specify a very tiny limit, the error will be delayed until
    // the first headers have been parsed by a call to lzma_code().
    std::string message;

    switch (initResult) {
      case LZMA_MEM_ERROR:
        message = "Memory allocation failed";
        break;

      case LZMA_OPTIONS_ERROR:
        message = "Unsupported decompressor flags";
        break;

      default:
        // This is most likely LZMA_PROG_ERROR indicating a bug in
        // this program or in liblzma. It is inconvenient to have a
        // separate error message for errors that should be impossible
        // to occur, but knowing the error code is important for
        // debugging. That's why it is good to print the error code
        // at least when there is no good error message to show.
        message = "Unknown error, possibly a bug";
        break;
    }

    std::cerr << "DECOMPRESSOR INIT ERROR: " << message << std::endl;

    return false;
  }

  bool uncompress(const unsigned char * inBuffer, const int inBufferSize,
                  unsigned char * outBuffer, int & outBufferSize)
  {
    std::shared_ptr<lzma_stream> stream = std::make_shared<lzma_stream>();

    if (!initDecoder(stream.get())) {
      return false;
    }

    bool retVal = false;

    // When LZMA_CONCATENATED flag was used when initializing the decoder,
    // we need to tell lzma_code() when there will be no more input.
    // This is done by setting action to LZMA_FINISH instead of LZMA_RUN
    // in the same way as it is done when encoding.
    //
    // When LZMA_CONCATENATED isn't used, there is no need to use
    // LZMA_FINISH to tell when all the input has been read, but it
    // is still OK to use it if you want. When LZMA_CONCATENATED isn't
    // used, the decoder will stop after the first .xz stream. In that
    // case some unused data may be left in strm->next_in.
    lzma_action action = LZMA_RUN;

    stream->next_in = nullptr;
    stream->avail_in = 0;
    stream->next_out = outBuffer;
    stream->avail_out = outBufferSize;

    int consumed = 0;

    while (true) {
      // Fill the input buffer if it is empty.
      if (stream->avail_in == 0) {
        if (consumed < inBufferSize) {
          if (inBufferSize - consumed > bufferSize) {
            stream->next_in = inBuffer + consumed;
            stream->avail_in = bufferSize;
          } else {
            stream->avail_in = inBufferSize - consumed;
          }

          consumed += stream->avail_in;
        }
        else {
          action = LZMA_FINISH;
        }          
      }

      lzma_ret uncompressionResult = lzma_code(stream.get(), action);

      // If the output buffer is full or if the compression finished
      // successfully, write the data from the output bufffer to
      // the output file.
      if (stream->avail_out == 0 and stream->avail_in > 0) {
        retVal = false;

        break;
      }

      if (uncompressionResult != LZMA_OK) {
        // Once everything has been encoded successfully, the
        // return value of lzma_code() will be LZMA_STREAM_END.
        //
        // It is important to check for LZMA_STREAM_END. Do not
        // assume that getting ret != LZMA_OK would mean that
        // everything has gone well.
        if (uncompressionResult == LZMA_STREAM_END) {
          outBufferSize -= stream->avail_out;
          retVal = true;

          break;
        }

        // It's not LZMA_OK nor LZMA_STREAM_END,
        // so it must be an error code. See lzma/base.h
        // (src/liblzma/api/lzma/base.h in the source package
        // or e.g. /usr/include/lzma/base.h depending on the
        // install prefix) for the list and documentation of
        // possible values. Many values listen in lzma_ret
        // enumeration aren't possible in this example, but
        // can be made possible by enabling memory usage limit
        // or adding flags to the decoder initialization.
        std::string message;

        switch (uncompressionResult) {
          case LZMA_MEM_ERROR:
            message = "Memory allocation failed";
            break;

          case LZMA_FORMAT_ERROR:
            // .xz magic bytes weren't found.
            message = "The input is not in the .xz format";
            break;

          case LZMA_OPTIONS_ERROR:
            // For example, the headers specify a filter
            // that isn't supported by this liblzma
            // version (or it hasn't been enabled when
            // building liblzma, but no-one sane does
            // that unless building liblzma for an
            // embedded system). Upgrading to a newer
            // liblzma might help.
            //
            // Note that it is unlikely that the file has
            // accidentally became corrupt if you get this
            // error. The integrity of the .xz headers is
            // always verified with a CRC32, so
            // unintentionally corrupt files can be
            // distinguished from unsupported files.
            message = "Unsupported compression options";
            break;

          case LZMA_DATA_ERROR:
            message = "Compressed file is corrupt";
            break;

          case LZMA_BUF_ERROR:
            // Typically this error means that a valid
            // file has got truncated, but it might also
            // be a damaged part in the file that makes
            // the decoder think the file is truncated.
            // If you prefer, you can use the same error
            // message for this as for LZMA_DATA_ERROR.
            message = "Compressed file is truncated or "
                "otherwise corrupt";
            break;

          default:
            // This is most likely LZMA_PROG_ERROR.
            message = "Unknown error, possibly a bug";
            break;
        }

        std::cerr << "UNCOMPRESSION ERROR: " << message
                  << " (error code " << uncompressionResult << ")"
                  << std::endl;

        break;
      }
    }

    lzma_end(stream.get());

    return retVal;
  }

}; // class LZMATest

TEST_F(LZMATest, CompressesAndUncompresses)
{
  for (int compressionLevel = 0; compressionLevel <= 9; compressionLevel++) {
    memset(compressedBuffer, 0, compressedCapacity);
    memset(uncompressedBuffer, 0, uncompressedCapacity);
    compressedSize = compressedCapacity;
    uncompressedSize = uncompressedCapacity;

    /* Compression */
    ASSERT_TRUE(compress(originalBuffer, originalData.size(),
                         compressedBuffer, compressedSize,
                         compressionLevel));
    EXPECT_LE(compressedSize, compressedCapacity);

    /* Decompression */
    ASSERT_TRUE(uncompress(compressedBuffer, compressedSize,
                           uncompressedBuffer, uncompressedSize));
    EXPECT_LE(uncompressedSize, uncompressedCapacity);
    EXPECT_EQ(originalData.size(), uncompressedSize);

    /* Integrity check */
    ASSERT_EQ(0, memcmp(originalBuffer, uncompressedBuffer,
                        originalData.size()));
  }
}

#endif //LZMA_TEST_H