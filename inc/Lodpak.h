#pragma once
#include "pch.h"
#include <map>


class Lodpack
{
	uint32_t groupCount;
	uint64_t* groupStartOff;
	uint64_t* groupHash;
	uint32_t* groupBlockSize;


	uint32_t TotalmembersCount;
	uint32_t* memberGroupIndex;
	uint32_t* memberOffsetter;
	uint64_t* memberHash;
	uint32_t* memberBlockSize;
public:
	fstream file;
	void Read(string filename);
	void Write(std::filesystem::path filename, std::map<uint64_t, std::stringstream*>& buffersHashmap);
	bool GetBuffer(const uint64_t& Hash, std::stringstream& outstream);
	bool SetBuffer(const uint64_t& Hash, std::iostream& stream);
};