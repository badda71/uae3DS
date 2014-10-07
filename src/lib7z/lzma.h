/* lzma.h -- LZMA Encoder
2008-08-05
Igor Pavlov
Public domain */

#ifndef __LZMA_H__
#define __LZMA_H__

#include "Types.h"

/*
.lzma header (13 bytes):
  Offset Size  Description
    0     1    lc, lp and pb in encoded form
    1     4    dictSize (little endian)
    5     8    uncompressed size (little endian)


Lzma_Encode
-------------
level - compression level: 0 <= level <= 9, the default value for "level" is 5.


dictSize - The dictionary size in bytes. The maximum value is
        128 MB = (1 << 27) bytes for 32-bit version
          1 GB = (1 << 30) bytes for 64-bit version
     The default value is 16 MB = (1 << 24) bytes, for level = 5.
     It's recommended to use the dictionary that is larger than 4 KB and
     that can be calculated as (1 << N) or (3 << N) sizes.
     For better compression ratio dictSize must be >= inSize.

LzmaEncode allocates Data with MyAlloc functions.
RAM Requirements for compressing:
  RamSize = dictionarySize * 11.5 + 6MB


Return code:
  SZ_OK               - OK
  SZ_ERROR_MEM        - Memory allocation error
  SZ_ERROR_PARAM      - Incorrect paramater
  SZ_ERROR_OUTPUT_EOF - output buffer overflow
  SZ_ERROR_THREAD     - errors in multithreading functions (only for Mt version)
*/


SRes Lzma_Encode(Byte *dest, size_t *destLen, const Byte *src, size_t srcLen,
    int level, UInt32 dictSize);

/*
Lzma86_Decode:
  In:
    dest     - output data
    destLen  - output data size
    src      - input data
    srcLen   - input data size
  Out:
    destLen  - processed output size
    srcLen   - processed input size
  Return code:
    SZ_OK           - OK
    SZ_ERROR_DATA  - Data error
    SZ_ERROR_MEM   - Memory allocation error
    SZ_ERROR_UNSUPPORTED - unsupported file
    SZ_ERROR_INPUT_EOF - it needs more bytes in input buffer
*/

SRes Lzma_Decode(Byte *dest, SizeT *destLen, const Byte *src, SizeT *srcLen);

#endif
