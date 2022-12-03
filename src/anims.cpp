#include "pch.h"
#include "anims.h"

//0xCD is autopad/uncleaned garbage.


struct UNKSTRUCT0
{
	uint16_t unk0;
	uint32_t unk1;
	uint16_t unk2;
	uint64_t offsetUnk;
	uint64_t Hash0Unk;
	uint64_t Hash1Unk;
};
struct UNKSTRUCT1
{
	uint32_t someCount;
	uint32_t unk0;
	uint32_t unk1;
	uint32_t unk3;
	char name[0x38];// NULL Terminated, following 0xCD are garbage
};
void AnimDef::Read(std::iostream &ms)
{
	uint64_t hash = 0;
	ms.seekg(0x18, ios::beg);
	ms.read((char*)&hash, sizeof hash);

	uint32_t unk0 = 0;
	ms.read((char*)&unk0, sizeof unk0);

	uint32_t unk1 = 0;
	ms.read((char*)&unk1, sizeof unk1);

	uint32_t fileSize = 0;
	ms.seekg(4, ios::cur);
	ms.read((char*)&fileSize, sizeof fileSize);

	uint16_t cnt0 = 0;
	ms.read((char*)&cnt0, sizeof cnt0);

	uint16_t cnt1 = 0;
	ms.read((char*)&cnt1, sizeof cnt1);


	uint32_t unk2 = 0;
	ms.read((char*)&unk2, sizeof unk2);

	ms.seekg(8, ios::cur);
	
	uint64_t offset0 = 0;
	ms.read((char*)&offset0, sizeof offset0);

	vector<uint64_t> offsetsUnkList(cnt1);
	for (auto i = 0; i < cnt1; i++)
	{
		ms.read((char*)&offsetsUnkList[i], sizeof uint64_t);
	}

	ms.seekg(offset0, ios::beg);
	vector<UNKSTRUCT0> unkStruct0List(cnt0);
	for (auto i = 0; i < cnt0; i++)
	{
		ms.read((char*)&unkStruct0List[i], sizeof UNKSTRUCT0);
	}

	vector<UNKSTRUCT1> unkStruct1List(cnt1);
	for (auto i = 0; i < cnt1; i++)
	{
		ms.seekg(offsetsUnkList[i], ios::beg);
		ms.read((char*)&unkStruct1List[i], sizeof UNKSTRUCT1);
	}
}
