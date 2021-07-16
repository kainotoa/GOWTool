#pragma once

#include "Mesh.h"

class Lodpack
{
	
public:
	uint32_t groupCount;
	uint32_t* groupStartOff;
	uint64_t* groupHash;
	uint32_t* groupBlockSize;

	
	uint32_t TotalmembersCount;
	uint32_t* memberGroupIndex;
	uint32_t* memberOffsetter;
	uint64_t* memberHash;
	uint32_t* memberBlockSize;
	
	void ReadLodpack(string filename);
};
class MGDefinition
{
public:
	uint16_t defCount;
	uint32_t* defOffsets;
	vector<MeshInfo> ReadMG(string filename);
	~MGDefinition();
};