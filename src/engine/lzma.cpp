#include "engine/lzma.h"
#include "engine/caaf.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <lzma.h>
#include <string>

namespace engine
{
namespace lzma
{

uint8_t *decompress(const uint8_t *data, size_t *size)
{
	lzma_stream strm = LZMA_STREAM_INIT;
	size_t srcsize = *size;

	// Get uncompressed file size:

	lzma_index *idx;
	lzma_ret ret = lzma_file_info_decoder(&strm, &idx, CAAF_DECOMP_MEMORY_MAX, srcsize);

	if (ret != LZMA_OK) {
		lzma_end(&strm);
		return nullptr;
	}

	strm.next_in = data;
	strm.avail_in = srcsize;
	ret = lzma_code(&strm, LZMA_RUN);

	if (ret != LZMA_STREAM_END) {
		lzma_end(&strm);
		return nullptr;
	}

	size_t ressize = lzma_index_uncompressed_size(idx); // Uncompressed file size
	idx = nullptr;

	// Decompress file (lzma frees everything from before):

	uint8_t *outbuf = new uint8_t[ressize];
	ret = lzma_stream_decoder(&strm, CAAF_DECOMP_MEMORY_MAX, 0);

	if (ret != LZMA_OK) {
		lzma_end(&strm);
		delete[] outbuf;
		return nullptr;
	}

	strm.next_in = data;
	strm.avail_in = srcsize;
	strm.next_out = outbuf;
	strm.avail_out = ressize;
	ret = lzma_code(&strm, LZMA_RUN);

	lzma_end(&strm);

	if (ret != LZMA_STREAM_END) {
		delete[] outbuf;
		return nullptr;
	}

	return outbuf;
}

#ifdef CAAF_ENABLE_DEBUG_TOOLS
bool compress(const uint8_t *data, size_t size, string fileout)
{
	ofstream fstrm(fileout);
	uint8_t outbuf[BUFSIZ];

	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_ret ret = lzma_easy_encoder(&strm, CAAF_LZMA_LEVEL, LZMA_CHECK_CRC64);

	if (ret != LZMA_OK) {
		lzma_end(&strm);
		return false;
	}

	strm.next_in = data;
	strm.avail_in = size;
	strm.next_out = outbuf;
	strm.avail_out = BUFSIZ;

	do {
		ret = lzma_code(&strm, LZMA_FINISH);
		if (ret != LZMA_OK || ret != LZMA_STREAM_END) break;

		fstrm.write((char *)outbuf, strm.avail_out);
	} while (ret == LZMA_OK);

	lzma_end(&strm);

	return ret == LZMA_STREAM_END;
}
#endif

} // namespace lzma
} // namespace engine