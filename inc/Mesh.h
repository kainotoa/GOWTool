#pragma once

#include "MathFunctions.h"

struct RawMeshContainer
{
	uint32_t VertCount{ 0 }, IndCount{ 0 };
	
	vector<Vec3> Positions;
	vector<Vec3> Normals;
	vector<Vec4> Tangents;
	
	vector<Vec2> TexCoord0;
	vector<Vec2> TexCoord1;
	vector<Vec2> TexCoord2;
	vector<Vec2> TexCoord3;
	
	vector<vector<uint16_t>> Joints;
	vector<vector<float>> Weights;
	
	vector<uint32_t> Indices;
	
	std::string Name;
	
	RawMeshContainer() = default;
};

// Range [0,17]
enum class PrimitiveTypes : uint8_t
{
	POSITION,
	NORMALS,
	TANGENTS,
	TEXCOORD_0,
	TEXCOORD_1,
	TEXCOORD_2,
	TEXCOORD_3,
	JOINTS0 = 9,
	WEIGHTS0
};
// Range [0,10] - { 5, 9 }
enum class DataTypes : uint8_t
{
	R32_FLOAT,
	R16_UNKNOWN, //Test Bits and use
	R32_UNKNOWN, //Test Use
	R10G10B10A2_TYPELESS, // Used as R10G10B10A2_UNORM [0, 1] and R10G10B10A2_SNORM [-1, 1]
	R16_UINT,
	R16_UNORM = 6,
	R16_SNORM,
	R8_UINT,
	R8_UNKNOWN = 10 //Test Bits and use
};

struct alignas (8) Component
{
	PrimitiveTypes primitiveType;
	DataTypes dataType;
	uint8_t elementCount;
	uint8_t offset;
	uint8_t bufferIndex;
};
struct MeshInfo
{
	uint64_t Hash{ 0 };
	
	uint32_t vertCount{ 0 };
	uint32_t indCount{ 0 };
	uint32_t faceCount{ 0 };

	uint64_t vertexOffset { 0 };
	uint64_t indicesOffset { 0 };

	vector<Component> Components;
	vector<uint64_t> bufferOffset;
	vector<uint16_t> bufferStride;

	Vec3 meshScale{ };
	Vec3 meshMin{ };
	
	uint16_t LODlvl{ 0 };
	
	uint16_t parentBone{ 0 };
	
	string name;

	uint8_t indicesStride = 2;
};