#pragma once

#include "MathFunctions.h"

struct RawMeshContainer
{
	uint32_t VertCount{ 0 }, IndCount{ 0 };
	Vec3* vertices{ nullptr };
	Vec3* normals{ nullptr };
	Vec4* tangents{ nullptr };
	Vec2* txcoord0{ nullptr };
	Vec2* txcoord1{ nullptr };
	Vec2* txcoord2{ nullptr };
	uint16_t* indices{ nullptr };
	uint16_t** joints{ nullptr };
	float** weights{ nullptr };
	std::string name;
	RawMeshContainer() = default;
};
enum class PrimitiveTypes
{
	POSITION,
	NORMALS,
	TANGENTS,
	TEXCOORD_0,
	TEXCOORD_1,
	TEXCOORD_2,
	UNKNOWN0,
	UNKNOWN1,
	UNKNOWN2,
	JOINTS0 = 9,
	WEIGHTS0,
	UNKNOWN3,
	UNKNOWN4,
	UNKNOWN5,
	UNKNOWN6,
	UNKNOWN7
};

enum class DataTypes
{
	FLOAT,
	HALFWORD_STRUCT_0,
	WORD_STRUCT_0,
	WORD_STRUCT_1,        // TEN TEN TEN 2 Normalized maybe? but sometimes its unsigned 1023 normalized (for weights) other times signed -511 shift 512 normalized 1's or 2's compliment maybe
	HALFWORD_STRUCT_1,   // USHORT un normalized maybe
	UNSIGNED_SHORT = 6,	// USHORT normalized
	HALFWORD_STRUCT_2,  //USHORT half normalized or compliment bs maybe
	BYTE_STRUCT_0		//UBYTE_UNORM
};
struct Component
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
	uint64_t vertexOffset{ UINT64_MAX };
	uint64_t indicesOffset{ UINT64_MAX };
	vector<Component> Components;
	vector<uint64_t> bufferOffset;
	vector<uint16_t> bufferStride;

	Vec3 meshScale{ };
	Vec3 meshMin{ };
	uint16_t LODlvl{ 0 };
	uint16_t boneAssociated{ 0 };
	string name;
};