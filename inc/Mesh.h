#pragma once

#include "MathFunctions.h"

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
		: VertCount { vertCount }
	    , IndCount { indCount }
		, vertices{ new Vec3[vertCount] }
		, normals{ new Vec3[vertCount] }
		, tangents{ new Vec4[vertCount] }
		, txcoord0{ new Vec2[vertCount] }
		, txcoord1{ new Vec2[vertCount] }
		, txcoord2{ new Vec2[vertCount] }
		, txcoord3{ new Vec2[vertCount] }
		, indices{ new uint16_t[indCount] }
		, joints{ new uint16_t*[indCount] }
		, weights{ new float*[vertCount] }
	{
		for (uint32_t i = 0; i < VertCount; i++)
			joints[i] = new uint16_t[4];
		for (uint32_t i = 0; i < VertCount; i++)
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
	uint64_t Hash { 0 };
	uint32_t vertCount { 0 };
	uint32_t indCount { 0 };
	uint16_t txcoordCount { 0 };
	uint32_t vertOffset { 0 };
	uint32_t indOffset { 0 };
	uint32_t norOffset { 0 };
	uint32_t tanOffset { 0 };
	vector<uint32_t> txcoordOffsets { };
	uint32_t jointOffset { 0 };
	uint32_t weightOffset { 0 };

	uint16_t Stride { 0 };
	uint16_t buffC { 0 };
	uint16_t compC { 0 };

	uint16_t vertSize { 0 };
	vector<uint16_t> txcoordSize { };
	uint16_t jointSize { 0 };

	Vec3 meshScale { };
	Vec3 meshMin { };

	uint16_t LODlvl { 0 };

	uint16_t norDetectCounter { 0 };
	uint16_t tanDetectCounter { 0 };
	uint16_t jointDetectCounter { 0 };
	uint16_t weightDetectCounter { 0 };
	vector<uint16_t> txcoordDetectCounter { };

	uint16_t boneAssociated { 0 };
};