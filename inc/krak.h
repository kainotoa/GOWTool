#pragma once
#include <pch.h>
#include <windows.h>

typedef int WINAPI OodLZ_CompressFunc(
	int codec, uint8* src_buf, size_t src_len, uint8* dst_buf, int level,
	void* opts, size_t offs, size_t unused, void* scratch, size_t scratch_size);

typedef int WINAPI OodLZ_DecompressFunc(uint8* src_buf, int src_len, uint8* dst, size_t dst_size,
	int fuzz, int crc, int verbose,
	uint8* dst_base, size_t e, void* cb, void* cb_ctx, void* scratch, size_t scratch_size, int threadPhase);



OodLZ_CompressFunc* OodLZ_Compress;
OodLZ_DecompressFunc* OodLZ_Decompress;

void LoadLib() {

#if defined(_M_X64)
#define LIBNAME "oo2core_7_win64.dll"
	char COMPFUNCNAME[] = "XXdleLZ_Compress";
	char DECFUNCNAME[] = "XXdleLZ_Decompress";
	COMPFUNCNAME[0] = DECFUNCNAME[0] = 'O';
	COMPFUNCNAME[1] = DECFUNCNAME[1] = 'o';
#else
#define LIBNAME "oo2core_7_win32.dll"
	char COMPFUNCNAME[] = "_XXdleLZ_Compress@40";
	char DECFUNCNAME[] = "_XXdleLZ_Decompress@56";
	COMPFUNCNAME[1] = DECFUNCNAME[1] = 'O';
	COMPFUNCNAME[2] = DECFUNCNAME[2] = 'o';
#endif
	HINSTANCE mod = LoadLibraryA(LIBNAME);
	OodLZ_Compress = (OodLZ_CompressFunc*)GetProcAddress(mod, COMPFUNCNAME);
	OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(mod, DECFUNCNAME);
	if (!OodLZ_Compress || !OodLZ_Decompress)
		cout << ("error loading", LIBNAME);
}