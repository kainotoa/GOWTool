#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Formats.h"
RawMeshContainer containRawMesh(MeshInfo& meshinfo, std::stringstream& file,std::string name, uint64_t off = 0);
void WriteRawMeshToStream(MeshInfo& meshInfo, const RawMeshContainer& rawMesh, std::iostream& outStream, const size_t& streamWriteOff = 0);