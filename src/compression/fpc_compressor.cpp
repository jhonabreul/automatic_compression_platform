/**
 *  AutoComp FPC Compressor
 *  lzma_compressor.cpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the FPC compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/15/2018
 */

#include "compression/fpc_compressor.hpp"

namespace autocomp {

// FPCCompressor constructor.
FPCCompressor::FPCCompressor(const int & compressionLevel)
  : LeveledCompressor(Compressor_Name(FPC), 1, 28, 20),
    BLOCK_SIZE(8 * 4096),
    mask{
      0x0000000000000000LL,
      0x00000000000000ffLL,
      0x000000000000ffffLL,
      0x0000000000ffffffLL,
      0x000000ffffffffffLL,
      0x0000ffffffffffffLL,
      0x00ffffffffffffffLL,
      0xffffffffffffffffLL
    }
{
  this->setCompressionLevel(compressionLevel);
}

// Compresses the data in the input buffer into the output buffer.
void FPCCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  this->_compress(inData, outData);
}

// Decompresses the data in the input buffer into the output buffer.
void FPCCompressor::decompress(const Buffer & inData, Buffer & outData) const
{
  this->_decompress(inData, outData);
}

// Compresses the data in the input buffer into the output buffer using the
// FPC compression algorithm
void FPCCompressor::_compress(const Buffer & inData, Buffer & outData) const
{
  const unsigned char * inBuffer;
  unsigned char * outBuffer;

  inBuffer = reinterpret_cast<const unsigned char *>(inData.getData());
  outBuffer = reinterpret_cast<unsigned char *>(outData.getData());

  register long i, out, intot, hash, dhash, code, bcode, ioc;
  register long long val, lastval, stride, pred1, pred2, xor1, xor2;
  register long long * fcm, * dfcm;
  unsigned long long inbuf[this->BLOCK_SIZE + 1];
  unsigned char outbuf[6 + (this->BLOCK_SIZE / 2) + (this->BLOCK_SIZE * 8) + 2];

  int availableOutBytes = outData.getCapacity();
  int inBufferSize = inData.getSize();

  long predsizem1 = this->compressionLevel;

  assert(0 == ((long)outbuf & 0x7));

  outbuf[0] = predsizem1;
  //ioc = fwrite(outbuf, 1, 1, stdout);
  //assert(1 == ioc);
  if (availableOutBytes == 0) {
    throw exceptions::CompressionError(this->compressorName,
                                       inData.getSize(),
                                       outData.getCapacity(),
                                       "Out buffer ran out of space");
  }
  memcpy(outBuffer, outbuf, 1);
  availableOutBytes -= 1;
  outBuffer += 1;
  predsizem1 = (1L << predsizem1) - 1;

  hash = 0;
  dhash = 0;
  lastval = 0;
  pred1 = 0;
  pred2 = 0;
  fcm = (long long *)calloc(predsizem1 + 1, 8);
  //fcm = new (std::nothrow) long long[predsizem1 + 2];
  assert(NULL != fcm);
  dfcm = (long long *)calloc(predsizem1 + 1, 8);
  //dfcm = new (std::nothrow) long long[predsizem1 + 2];
  assert(NULL != dfcm);

  //intot = fread(inbuf, 8, this->BLOCK_SIZE, stdin);
  intot = inBufferSize < 8 * this->BLOCK_SIZE
            ? inBufferSize
            : 8 * this->BLOCK_SIZE;
  memcpy(inbuf, inBuffer, intot);
  inBufferSize -= intot;
  if (inBufferSize != 0) {
    inBuffer += intot;
  }
  intot /= 8;

  while (0 < intot) {
    val = inbuf[0];
    out = 6 + ((intot + 1) >> 1);
    *((long long *)&outbuf[(out >> 3) << 3]) = 0;
    for (i = 0; i < intot; i += 2) {
      xor1 = val ^ pred1;
      fcm[hash] = val;
      hash = ((hash << 6) ^ ((unsigned long long)val >> 48)) & predsizem1;
      pred1 = fcm[hash];

      stride = val - lastval;
      xor2 = val ^ (lastval + pred2);
      lastval = val;
      val = inbuf[i + 1];
      dfcm[dhash] = stride;
      dhash = ((dhash << 2) ^ ((unsigned long long)stride >> 40)) & predsizem1;
      pred2 = dfcm[dhash];

      code = 0;
      if ((unsigned long long)xor1 > (unsigned long long)xor2) {
        code = 0x80;
        xor1 = xor2;
      }
      bcode = 7;                // 8 bytes
      if (0 == (xor1 >> 56)) {
        bcode = 6;              // 7 bytes
      }
      if (0 == (xor1 >> 48)) {
        bcode = 5;              // 6 bytes
      }
      if (0 == (xor1 >> 40)) {
        bcode = 4;              // 5 bytes
      }
      if (0 == (xor1 >> 24)) {
        bcode = 3;              // 3 bytes
      }
      if (0 == (xor1 >> 16)) {
        bcode = 2;              // 2 bytes
      }
      if (0 == (xor1 >> 8)) {
        bcode = 1;              // 1 byte
      }
      if (0 == xor1) {
        bcode = 0;              // 0 bytes
      }

      *((long long *)&outbuf[(out >> 3) << 3]) |= xor1 << ((out & 0x7) << 3);
      if (0 == (out & 0x7)) {
        xor1 = 0;
      }
      *((long long *)&outbuf[((out >> 3) << 3) + 8]) =
          (unsigned long long)xor1 >> (64 - ((out & 0x7) << 3));

      out += bcode + (bcode >> 2);
      code |= bcode << 4;

      xor1 = val ^ pred1;
      fcm[hash] = val;
      hash = ((hash << 6) ^ ((unsigned long long)val >> 48)) & predsizem1;
      pred1 = fcm[hash];

      stride = val - lastval;
      xor2 = val ^ (lastval + pred2);
      lastval = val;
      val = inbuf[i + 2];
      dfcm[dhash] = stride;
      dhash = ((dhash << 2) ^ ((unsigned long long)stride >> 40)) & predsizem1;
      pred2 = dfcm[dhash];

      bcode = code | 0x8;
      if ((unsigned long long)xor1 > (unsigned long long)xor2) {
        code = bcode;
        xor1 = xor2;
      }
      bcode = 7;                // 8 bytes
      if (0 == (xor1 >> 56)) {
        bcode = 6;              // 7 bytes
      }
      if (0 == (xor1 >> 48)) {
        bcode = 5;              // 6 bytes
      }
      if (0 == (xor1 >> 40)) {
        bcode = 4;              // 5 bytes
      }
      if (0 == (xor1 >> 24)) {
        bcode = 3;              // 3 bytes
      }
      if (0 == (xor1 >> 16)) {
        bcode = 2;              // 2 bytes
      }
      if (0 == (xor1 >> 8)) {
        bcode = 1;              // 1 byte
      }
      if (0 == xor1) {
        bcode = 0;              // 0 bytes
      }

      *((long long *)&outbuf[(out >> 3) << 3]) |= xor1 << ((out & 0x7) << 3);
      if (0 == (out & 0x7)) {
        xor1 = 0;
      }
      *((long long *)&outbuf[((out >> 3) << 3) + 8]) =
          (unsigned long long)xor1 >> (64 - ((out & 0x7) << 3));

      out += bcode + (bcode >> 2);
      outbuf[6 + (i >> 1)] = code | bcode;
    }

    if (0 != (intot & 1)) {
      out -= bcode + (bcode >> 2);
    }

    outbuf[0] = intot;
    outbuf[1] = intot >> 8;
    outbuf[2] = intot >> 16;
    outbuf[3] = out;
    outbuf[4] = out >> 8;
    outbuf[5] = out >> 16;
    //ioc = fwrite(outbuf, 1, out, stdout);
    //assert(ioc == out);
    if (availableOutBytes < out) {
      //delete[] fcm;
      //delete[] dfcm;
      free(fcm);
      free(dfcm);
      throw exceptions::CompressionError(this->compressorName,
                                         inData.getSize(),
                                         outData.getCapacity(),
                                         "Out buffer ran out of space");
    }
    memcpy(outBuffer, outbuf, out);
    availableOutBytes -= out;
    if (availableOutBytes != 0) {
      outBuffer += out;
    }
    //intot = fread(inbuf, 8, this->BLOCK_SIZE, stdin);
    intot = inBufferSize < 8 * this->BLOCK_SIZE
              ? inBufferSize
              : 8 * this->BLOCK_SIZE;
    memcpy(inbuf, inBuffer, intot);
    inBufferSize -= intot;
    if (inBufferSize != 0) {
      inBuffer += intot;
    }
    intot /= 8;
  }

  //delete[] fcm;
  //delete[] dfcm;
  free(fcm);
  free(dfcm);

  try {
    outData.setSize(outData.getCapacity() - availableOutBytes);
  }
  catch (std::domain_error & error) {
    throw exceptions::CompressionError(this->compressorName,
                                       inData.getSize(),
                                       outData.getCapacity(),
                                       error.what());
  }
}

// Decompresses the data in the input buffer into the output buffer using the
// FPC compression algorithm
void FPCCompressor::_decompress(const Buffer & inData, Buffer & outData) const
{

  const unsigned char * inBuffer;
  unsigned char * outBuffer;

  inBuffer = reinterpret_cast<const unsigned char *>(inData.getData());
  outBuffer = reinterpret_cast<unsigned char *>(outData.getData());

  register long in, intot, hash, dhash, code, bcode, predsizem1, end, tmp, ioc;
  register long long val, lastval, stride, pred1, pred2, next;
  register long long * fcm, * dfcm;
  long long outbuf[this->BLOCK_SIZE];
  unsigned char inbuf[(this->BLOCK_SIZE / 2) + (this->BLOCK_SIZE * 8) + 6 + 2];

  int availableOutBytes = outData.getCapacity();
  int inBufferSize = inData.getSize();

  assert(0 == ((long)inbuf & 0x7));

  //ioc = fread(inbuf, 1, 7, stdin);
  if (inBufferSize < 7) {
    throw exceptions::DecompressionError(this->compressorName,
                                       inData.getSize(),
                                       outData.getCapacity(),
                                       "In boffer is not in FPC format");
  }
  ioc = 7;
  memcpy(inbuf, inBuffer, ioc);
  inBufferSize -= ioc;
  if (inBufferSize != 0) {
    inBuffer += ioc;
  }

  if (1 != ioc) {
    assert(7 == ioc);
    predsizem1 = inbuf[0];
    predsizem1 = (1L << predsizem1) - 1;

    hash = 0;
    dhash = 0;
    lastval = 0;
    pred1 = 0;
    pred2 = 0;
    fcm = (long long *)calloc(predsizem1 + 1, 8);
    //fcm = new (std::nothrow) long long[predsizem1 + 2];
    assert(NULL != fcm);
    dfcm = (long long *)calloc(predsizem1 + 1, 8);
    //dfcm = new (std::nothrow) long long[predsizem1 + 2];
    assert(NULL != dfcm);

    intot = inbuf[3];
    intot = (intot << 8) | inbuf[2];
    intot = (intot << 8) | inbuf[1];
    in = inbuf[6];
    in = (in << 8) | inbuf[5];
    in = (in << 8) | inbuf[4];
    assert(this->BLOCK_SIZE >= intot);
    do {
      //end = fread(inbuf, 1, in, stdin);
      if (inBufferSize < in) {
        end = inBufferSize;
      }
      else {
        end = in;
      }
      memcpy(inbuf, inBuffer, end);
      inBufferSize -= end;
      if (inBufferSize != 0) {
        inBuffer += end;
      }

      assert((end + 6) >= in);
      in = (intot + 1) >> 1;
      for (int i = 0; i < intot; i += 2) {
        code = inbuf[i >> 1];

        val = *((long long *)&inbuf[(in >> 3) << 3]);
        next = *((long long *)&inbuf[((in >> 3) << 3) + 8]);
        tmp = (in & 0x7) << 3;
        val = (unsigned long long)val >> tmp;
        next <<= 64 - tmp;
        if (0 == tmp) {
          next = 0;
        }
        val |= next;

        bcode = (code >> 4) & 0x7;
        val &= mask[bcode];
        in += bcode + (bcode >> 2);

        if (0 != (code & 0x80)) {
          pred1 = pred2;
        }
        val ^= pred1;

        fcm[hash] = val;
        hash = ((hash << 6) ^ ((unsigned long long)val >> 48)) & predsizem1;
        pred1 = fcm[hash];

        stride = val - lastval;
        dfcm[dhash] = stride;
        dhash = ((dhash << 2) ^ ((unsigned long long)stride >> 40)) & predsizem1;
        pred2 = val + dfcm[dhash];
        lastval = val;

        outbuf[i] = val;

        val = *((long long *)&inbuf[(in >> 3) << 3]);
        next = *((long long *)&inbuf[((in >> 3) << 3) + 8]);
        tmp = (in & 0x7) << 3;
        val = (unsigned long long)val >> tmp;
        next <<= 64 - tmp;
        if (0 == tmp) {
          next = 0;
        }
        val |= next;

        bcode = code & 0x7;
        val &= mask[bcode];
        in += bcode + (bcode >> 2);

        if (0 != (code & 0x8)) {
          pred1 = pred2;
        }
        val ^= pred1;

        fcm[hash] = val;
        hash = ((hash << 6) ^ ((unsigned long long)val >> 48)) & predsizem1;
        pred1 = fcm[hash];

        stride = val - lastval;
        dfcm[dhash] = stride;
        dhash = ((dhash << 2) ^ ((unsigned long long)stride >> 40)) & predsizem1;
        pred2 = val + dfcm[dhash];
        lastval = val;

        outbuf[i + 1] = val;
      }
      //ioc = fwrite(outbuf, 8, intot, stdout);
      ioc = 8 * intot;
      if (availableOutBytes < ioc) {
        //delete[] fcm;
        //delete[] dfcm;
        free(fcm);
        free(dfcm);
        throw exceptions::DecompressionError(this->compressorName,
                                       inData.getSize(),
                                       outData.getCapacity(),
                                       "Out buffer ran out of space");
      }
      memcpy(outBuffer, outbuf, ioc);
      availableOutBytes -= ioc;
      if (availableOutBytes != 0) {
        outBuffer += ioc;
      }
      ioc /= 8;
      assert(ioc == intot);
      intot = 0;
      if ((end - 6) >= in) {
        intot = inbuf[in + 2];
        intot = (intot << 8) | inbuf[in + 1];
        intot = (intot << 8) | inbuf[in];
        end = inbuf[in + 5];
        end = (end << 8) | inbuf[in + 4];
        end = (end << 8) | inbuf[in + 3];
        in = end;
      }
      assert(this->BLOCK_SIZE >= intot);
    } while (0 < intot);

    //delete[] fcm;
    //delete[] dfcm;
    free(fcm);
    free(dfcm);

    try {
      outData.setSize(outData.getCapacity() - availableOutBytes);
    }
    catch (std::domain_error & error) {
      throw exceptions::DecompressionError(this->compressorName,
                                         inData.getSize(),
                                         outData.getCapacity(),
                                         error.what());
    }
  }
}

} // namespace autocomp