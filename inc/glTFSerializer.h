#pragma once
#include "pch.h"
#include "Mesh.h"

void WriteGLTF(const std::filesystem::path& path, const vector<RawMesh>& expMeshes);