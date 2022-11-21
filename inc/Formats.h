#pragma once
#include "Mesh.h"

class MGDefinition
{
	uint16_t defCount;
	uint32_t* defOffsets;
public:
	vector<MeshInfo> ReadMG(std::stringstream& file);
	bool WriteMG(const std::vector<MeshInfo>& meshinfos, std::stringstream& file);
	~MGDefinition();
};
class SmshDefinition
{
public:
	vector<MeshInfo> ReadSmsh(std::iostream& file);
};

namespace GOWR
{
	class MESH
	{
	public:
		static bool Parse(std::iostream& stream, vector<MeshInfo>& meshes, size_t endOffOrSize , size_t baseOff = 0);
	};
}