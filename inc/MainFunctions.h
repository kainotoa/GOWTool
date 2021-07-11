#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Formats.h"
std::vector<MeshInfo> getMeshesInfo(MGDefinition mg, std::string filename);
RawMesh containRawMesh(MeshInfo meshinfo, std::ifstream& file, uint32_t off = 0);