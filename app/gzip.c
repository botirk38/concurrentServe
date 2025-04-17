#include "gzip.h"
#include <zlib.h>

int gzip_compress(const char *src, int srcLen, char *dst, int dstLen) {
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  // 31 = 16 (gzip) + 15 (window bits)
  if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                   Z_DEFAULT_STRATEGY) != Z_OK) {
    return -1;
  }

  strm.avail_in = srcLen;
  strm.next_in = (Bytef *)src;
  strm.avail_out = dstLen;
  strm.next_out = (Bytef *)dst;

  int ret = deflate(&strm, Z_FINISH);
  deflateEnd(&strm);

  if (ret != Z_STREAM_END) {
    return -1;
  }

  return dstLen - strm.avail_out;
}
