#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Formats.h"
RawMeshContainer containRawMesh(MeshInfo& meshinfo, std::stringstream& file,std::string name, uint64_t off = 0);