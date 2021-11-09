#include "pch.h"
#include "anims.h"

void AnimDef::Read(std::iostream &ms)
{
	uint16_t unk0Cnt{ 0 };
	ms.seekg(0x32, ios::beg);
	ms.read((char*)&unk0Cnt, sizeof(unk0Cnt));
	uint64_t* unk0Offs = new uint64_t[unk0Cnt];

}
