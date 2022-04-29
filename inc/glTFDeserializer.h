#pragma once

#include "pch.h"
#include "Mesh.h"
#include "Rig.h"
#include <GLTFSDK/Document.h>
#include <GLTFSDK/GLTFResourceReader.h>


bool ReadGLTF(const std::filesystem::path& path, Microsoft::glTF::Document& document, std::shared_ptr<Microsoft::glTF::GLTFResourceReader>& resourceReader);
void PrintDocumentInfo(const Microsoft::glTF::Document& document);
bool GetRawMeshesFromGLTF(const Microsoft::glTF::Document& document, const Microsoft::glTF::GLTFResourceReader& resourceReader, std::vector<RawMeshContainer>& Meshes, const Rig& Armature);