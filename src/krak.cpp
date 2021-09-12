#include "pch.h"
#include "krak.h"

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