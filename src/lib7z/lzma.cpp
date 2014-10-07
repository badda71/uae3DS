/* Lzma86Enc.c -- LZMA + x86 (BCJ) Filter Encoder
2008-08-05
Igor Pavlov
Public domain */

#include <string.h>

#include "lzma.h"

#include "Alloc.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"

#define SZE_OUT_OVERFLOW SZE_DATA_ERROR
#define LZMA_SIZE_OFFSET LZMA_PROPS_SIZE
#define LZMA_HEADER_SIZE (LZMA_SIZE_OFFSET + 8)

static void *SzAlloc(void *p, size_t size) { p = p; return MyAlloc(size); }
static void SzFree(void *p, void *address) { p = p; MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

#define LZMA_SIZE_OFFSET LZMA_PROPS_SIZE
#define LZMA_HEADER_SIZE (LZMA_SIZE_OFFSET + 8)

int Lzma_Encode(Byte *dest, size_t *destLen, const Byte *src, size_t srcLen,
    int level, UInt32 dictSize)
{
	size_t outSize2 = *destLen;
	int mainResult = SZ_ERROR_OUTPUT_EOF;
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level = level;
	props.dictSize = dictSize;

	*destLen = 0;
	if (outSize2 < LZMA_HEADER_SIZE)
		return SZ_ERROR_OUTPUT_EOF;

	/* Store the input file size in the output stream */
	{
		int i;
		UInt64 t = srcLen;
		for (i = 0; i < 8; i++, t >>= 8)
		  dest[LZMA_SIZE_OFFSET + i] = (Byte)t;
	}


  {
    size_t minSize = 0;

    {
      size_t outSizeProcessed = outSize2 - LZMA_HEADER_SIZE;
      size_t outPropsSize = 5;
      SRes curRes;

      curRes = LzmaEncode(dest + LZMA_HEADER_SIZE, &outSizeProcessed,
          src, srcLen,
          &props, dest, &outPropsSize, 0,
          NULL, &g_Alloc, &g_Alloc);

      if (curRes != SZ_ERROR_OUTPUT_EOF)
      {
        if (curRes != SZ_OK)
        {
          return curRes;
        }
        if (outSizeProcessed <= minSize || mainResult != SZ_OK)
        {
          minSize = outSizeProcessed;
          mainResult = SZ_OK;
        }
      }
    }

    *destLen = LZMA_HEADER_SIZE + minSize;
  }
  return mainResult;
}


SRes Lzma_Decode(Byte *dest, SizeT *destLen, const Byte *src, SizeT *srcLen)
{
  SRes res;
  SizeT inSizePure;
  ELzmaStatus status;

  if (*srcLen < LZMA_HEADER_SIZE)
    return SZ_ERROR_INPUT_EOF;

  inSizePure = *srcLen - LZMA_HEADER_SIZE;
  res = LzmaDecode(dest, destLen, src + LZMA_HEADER_SIZE, &inSizePure,
      src, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &g_Alloc);
  *srcLen = inSizePure + LZMA_HEADER_SIZE;

  return SZ_OK;
}
