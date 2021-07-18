#pragma once
#include "pch.h"

class Lodpack
{
	uint32_t groupCount;
	uint32_t* groupStartOff;
	uint64_t* groupHash;
	uint32_t* groupBlockSize;


	uint32_t TotalmembersCount;
	uint32_t* memberGroupIndex;
	uint32_t* memberOffsetter;
	uint64_t* memberHash;
	uint32_t* memberBlockSize;
public:
	ifstream file;
	Lodpack(string filename);
	bool GetBuffer(uint64_t& Hash, std::stringstream& outstream);
};