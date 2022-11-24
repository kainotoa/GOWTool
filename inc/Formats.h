#pragma once
#include "Mesh.h"

namespace GOWR
{
	class MESH
	{
	public:
		static bool Parse(std::iostream& stream, vector<MeshInfo>& meshes, size_t endOffOrSize , size_t baseOff = 0);
	};
	class MG
	{
	public:
		static bool Parse(std::iostream& stream, vector<MeshInfo>& meshes, size_t mgBufferSize, size_t baseOff = 0);
	};
}