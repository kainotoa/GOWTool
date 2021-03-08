#pragma once
#include <3DStructs.h>
#include <vector>
struct RawMesh
{
	uint32_t VertCount, IndCount;
	Vec3* vertices;
	Vec3* normals;
	Vec4* tangents;
	Vec2* txcoord0;
	Vec2* txcoord1;
	Vec2* txcoord2;
	Vec2* txcoord3;
	uint16_t* indices;
	uint16_t** joints;
	float** weights;
	RawMesh(uint32_t vertCount, uint32_t indCount)
	{
		VertCount = vertCount;
		IndCount = indCount;
		vertices = new Vec3[VertCount];
		normals = new Vec3[VertCount];
		tangents = new Vec4[VertCount];
		txcoord0 = new Vec2[VertCount];
		txcoord1 = new Vec2[VertCount];
		txcoord2 = new Vec2[VertCount];
		txcoord3 = new Vec2[VertCount];
		indices = new uint16_t[IndCount];

		joints = new uint16_t* [VertCount];
		for (int i = 0; i < VertCount; i++)
			joints[i] = new uint16_t[4];
		weights = new float* [VertCount];
		for (int i = 0; i < VertCount; i++)
			weights[i] = new float[4];
	}
};
enum ComponentTypes
{
	POSITION, NORMALS, TANGENTS, TEXCOORD_0, TEXCOORD_1, TEXCOORD_2, JOINTS0 = 9, WEIGHTS0
};
struct MeshComp
{
	ComponentTypes compID;
	uint16_t dataType;
	uint16_t elemCount;
	uint16_t strider;
	uint16_t index;
};
struct MeshInfo
{
	uint64_t Hash;
	uint32_t vertCount;
	uint32_t indCount;
	uint16_t txcoordCount;
	uint32_t vertOffset;
	uint32_t indOffset;
	uint32_t norOffset;
	uint32_t tanOffset;
	std::vector<uint32_t> txcoordOffsets;
	uint32_t jointOffset;
	uint32_t weightOffset;

	uint16_t Stride;
	uint16_t buffC;
	uint16_t compC;

	uint16_t vertSize;
	std::vector<uint16_t> txcoordSize;
	uint16_t jointSize;

	Vec3 meshScale;
	Vec3 meshMin;

	uint16_t LODlvl;

	uint16_t norDetectCounter;
	uint16_t tanDetectCounter;
	uint16_t jointDetectCounter;
	uint16_t weightDetectCounter;
	std::vector<uint16_t> txcoordDetectCounter;

	uint16_t boneAssociated;
	MeshInfo(void)
	{
		Hash = 0;
		vertCount = 0;
		indCount = 0;
		txcoordCount = 0;
		vertOffset = 0;
		indOffset = 0;
		norOffset = 0;
		tanOffset = 0;
		jointOffset = 0;
		weightOffset = 0;

		Stride = 0;
		buffC = 0;
		compC = 0;

		vertSize = 0;
		jointSize = 0;
		LODlvl = 0;
		norDetectCounter = 0;
		tanDetectCounter = 0;
		jointDetectCounter = 0;
		weightDetectCounter = 0;
		boneAssociated = 0;
	}
};