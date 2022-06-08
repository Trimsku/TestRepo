#include <cstdio>
#include <zlib.h>
#include "ZlibApi.h"

// Functions are taken
// from https://zlib.net/zpipe.c and modified by trimsky

int doDeflate(FILE *source, FILE *dest, int level) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (std::ferror(source)) {
            deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = std::feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (std::fwrite(out, 1, have, dest) != have || std::ferror(dest)) {
                deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    deflateEnd(&strm);
    return Z_OK;
}

int doInflate(FILE *source, FILE *dest) {
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (std::ferror(source)) {
            inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            if (std::fwrite(out, 1, have, dest) != have || std::ferror(dest)) {
                inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void checkZlibError(int ret)
{
    std::cerr << "archiver: ";
    switch (ret) {
        case Z_ERRNO:
            if (std::ferror(stdin))
                std::cerr << "error reading stdin\n";
            if (std::ferror(stdout))
                std::cerr << "error writing stdout\n";
            break;
        case Z_STREAM_ERROR:
            std::cerr << "invalid compression level\n";
            break;
        case Z_DATA_ERROR:
            std::cerr << "invalid or incomplete deflate data\n";
            break;
        case Z_MEM_ERROR:
            std::cerr << "out of memory\n";
            break;
        case Z_VERSION_ERROR:
            std::cerr << "zlib version mismatch!\n";
    }
}