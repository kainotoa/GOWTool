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

	uint32_t meshCount;
	vector<uint32_t> indicesOffsetter;
	vector<uint32_t> verticesOffsetter;
	vector<uint32_t> indicesCount;
	vector<uint32_t> verticesCount;
	vector<Vec3> meshScale;
	vector<Vec3> meshMin;
	vector<uint32_t> vertexBlockOffsetter;
	vector<uint64_t> meshHash;
	vector<uint16_t> buffCount;
	vector<uint16_t> CompCount;
	vector<vector<MeshComp>> Components;

	vector<uint16_t> LODlvl;
	vector<uint16_t> boneAssociated;
	void ReadMG(string filename);
};