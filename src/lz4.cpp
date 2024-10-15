#include "lz4.h"
#include "pch.h"
#include <lz4\lib\lz4frame.h>

using std::stringstream;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;


shared_ptr<iostream> DecompressWad(shared_ptr<iostream> stream)
{
	size_t size;
	stream->seekg(0, std::ios::end);
	size = stream->tellg();


	unique_ptr<char[]> src = make_unique<char[]>(size);
	stream->seekg(0x0, std::ios::beg);
	stream->read(src.get(), size);


	size_t dstCapacity = 0;
	stream->seekg(0x6, std::ios::beg);
	stream->read((char*)&dstCapacity, 4);
	unique_ptr<char[]> dst = make_unique<char[]>(dstCapacity);


	LZ4F_dctx* ctx;
	LZ4F_createDecompressionContext(&ctx, LZ4F_getVersion());

	LZ4F_decompressOptions_t opn = { 0,0,0,0 };
	LZ4F_decompress(ctx, dst.get(), &dstCapacity, src.get(), &size, &opn);


	shared_ptr<stringstream> ss = make_shared<stringstream>();
	ss->write(dst.get(), dstCapacity);


	ss->seekg(0, std::ios::beg);
	return ss;
}