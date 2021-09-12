#pragma once

#include <pch.h>
#include <windows.h>

#define SAFE_SPACE 64

typedef unsigned char uint8;

typedef int WINAPI OodLZ_CompressFunc(
	int codec, uint8* src_buf, size_t src_len, uint8* dst_buf, int level,
	void* opts, size_t offs, size_t unused, void* scratch, size_t scratch_size);

typedef int WINAPI OodLZ_DecompressFunc(uint8* src_buf, int src_len, uint8* dst, size_t dst_size,
	int fuzz, int crc, int verbose,
	uint8* dst_base, size_t e, void* cb, void* cb_ctx, void* scratch, size_t scratch_size, int threadPhase);

extern OodLZ_CompressFunc* OodLZ_Compress;
extern OodLZ_DecompressFunc* OodLZ_Decompress;

void LoadLib();