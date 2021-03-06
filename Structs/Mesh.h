#include <3DStructs.h>
struct RawMesh
{
	uint32_t VertCount, IndCount;
	Vec3* vertices;
	Vec3* normals;
	Vec4* tangents;
	Vec2* txcoord0;
	Vec2* txcoord1;
	uint16_t* indices;
	RawMesh(uint32_t vertCount, uint32_t indCount)
	{
		VertCount = vertCount;
		IndCount = indCount;
		vertices = new Vec3[VertCount];
		normals = new Vec3[VertCount];
		tangents = new Vec4[VertCount];
		txcoord0 = new Vec2[VertCount];
		txcoord1 = new Vec2[VertCount];
		indices = new uint16_t[IndCount];
	}
};