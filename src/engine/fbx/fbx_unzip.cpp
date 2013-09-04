
#include "fbx_unzip.hpp"

extern "C" {
	#include <string.h>
#ifdef USE_ZLIB
	#include <zlib.h>
#else
	// Define MINIZ_NO_STDIO to disable all usage and any functions which rely on stdio for file I/O.
	#define MINIZ_NO_STDIO

	// If MINIZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or
	// get/set file times.
	#define MINIZ_NO_TIME

	// Define MINIZ_NO_ARCHIVE_APIS to disable all ZIP archive API's.
	#define MINIZ_NO_ARCHIVE_APIS

	// Define MINIZ_NO_ARCHIVE_APIS to disable all writing related ZIP archive API's.
	#define MINIZ_NO_ARCHIVE_WRITING_APIS

	// Define MINIZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression API's.
	//#define MINIZ_NO_ZLIB_APIS

	// Define MINIZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent conflicts against stock zlib.
	#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

	#include "miniz.c"

	// #define's are dangerous. in this case, they'd rename FBX::inflate to FBX::mz_inflate.
	// use proper aliasing instead.
	typedef mz_stream z_stream;
	static int inflateInit(z_stream* pStream) { return ::mz_inflateInit(pStream); }
	static int inflateEnd(z_stream* pStream) { return ::mz_inflateEnd(pStream); }
	static int inflate(z_stream* pStream, int flush) { return ::mz_inflate(pStream, flush); }

	// these constants should be safe to #define - uppercase names are expected to be #define'd
	#define Z_NO_FLUSH            MZ_NO_FLUSH
	#define Z_FINISH              MZ_FINISH
	#define Z_OK                  MZ_OK
	#define Z_STREAM_END          MZ_STREAM_END
	#define Z_NEED_DICT           MZ_NEED_DICT
	#define Z_STREAM_ERROR        MZ_STREAM_ERROR
	#define Z_DATA_ERROR          MZ_DATA_ERROR
	#define Z_MEM_ERROR           MZ_MEM_ERROR
	#define Z_BUF_ERROR           MZ_BUF_ERROR
#endif
}

namespace FBX {
	std::vector<uint8_t> inflate(const std::vector<uint8_t> &data, size_t out_hint) {
		z_stream strm;
		int ret;
		memset(&strm, 0, sizeof(strm));
		uint8_t out[4096];
		std::vector<uint8_t> result;
		bool appending = false;
		result.resize(out_hint);

		ret = ::inflateInit(&strm);
		if (ret != Z_OK) throw InflateException("inflateInit failed");

		strm.avail_in = (unsigned int) data.size();
		strm.next_in = const_cast<unsigned char*>(data.data());

		strm.avail_out = result.size();
		strm.next_out = result.data();

		for (;;) {
			bool finish = strm.avail_in == 0;
			ret = ::inflate(&strm, finish ? Z_FINISH : Z_NO_FLUSH);
			if (ret != Z_OK && ret != Z_BUF_ERROR) break;

			if (appending) {
				if (ret == Z_BUF_ERROR && strm.next_out == out) {
					inflateEnd(&strm);
					throw InflateException("inflate: no progress");
				}

				result.insert(result.end(), out, strm.next_out);

				strm.avail_out = sizeof(out);
				strm.next_out = out;
			} else if (strm.avail_out == 0 && (finish || strm.avail_in != 0)) {
				appending = true;
				result.resize(strm.total_out);
				strm.avail_out = sizeof(out);
				strm.next_out = out;
			}
		}
		if (appending) {
			result.insert(result.end(), out, strm.next_out);
		} else {
			result.resize(strm.total_out);
		}

		::inflateEnd(&strm);

		switch (ret) {
		case Z_STREAM_END:
			break;
		case Z_STREAM_ERROR:
			throw InflateException("inflate: stream error");
		case Z_NEED_DICT:
			throw InflateException("inflate: need dict");
		case Z_DATA_ERROR:
			throw InflateException("inflate: data error");
		case Z_MEM_ERROR:
			throw InflateException("inflate: memory error");
		default:
			throw InflateException("inflate: unknown error");
		}

		return result;
	}
}
