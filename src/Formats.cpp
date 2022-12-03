#include <pch.h>
#include "Formats.h"
#include <cassert>
#include <map>
#include <set>
#include <utility>

std::map<PrimitiveTypes, std::set<std::pair<DataTypes, int>>> compMap;
std::set<DataTypes> dataSet;
namespace GOWR
{
	bool MESH::Parse(std::iostream& stream, vector<MeshInfo>& meshes, size_t meshBufferSize, size_t baseOff)
	{
		// Checks
		if (meshBufferSize < 0x20)
			return false;
		stream.seekg(0, std::ios::end);
		if (stream.tellg() < meshBufferSize)
			return false;

		uint32_t defOffsetsBeg{ 0x34 };
		stream.seekg(baseOff + 0xC);
		stream.read((char*)&defOffsetsBeg,sizeof defOffsetsBeg);

		uint32_t defCount{ 0 };
		stream.read((char*)&defCount, sizeof defCount);

		uint32_t fileSize{ 0 };
		stream.seekg(0x8, ios::cur);
		stream.read((char*)&fileSize, sizeof fileSize);

		uint32_t defOff = 0;
		
		if (meshBufferSize < (defCount * sizeof defOff + defOffsetsBeg + 0xC))
			return false;

		baseOff += defOffsetsBeg + 0xC;
		for (uint32_t i = 0; i < defCount; i++)
		{
			MeshInfo info;
			
			stream.seekg(baseOff + i * sizeof defOff);

			stream.read((char*)&defOff, sizeof defOff);

			stream.seekg(baseOff + i * sizeof defOff + defOff);

			stream.seekg(0x10, ios::cur);

			Vec3 extent;
			stream.read((char*)&extent, sizeof extent);

			Vec3 origin;
			stream.read((char*)&origin, sizeof origin);

			info.meshScale = Vec3(extent.X * 2, extent.Y * 2, extent.Z * 2);
			info.meshMin =  Vec3(origin.X - extent.X, origin.Y - extent.Y, origin.Z - extent.Z);

			stream.seekg(0x8, ios::cur);
			stream.read((char*)&info.indicesOffset, sizeof uint32_t);
			
			stream.seekg(0x8, ios::cur);
			stream.read((char*)&info.vertexOffset, sizeof uint32_t);

			stream.seekg(0x4, ios::cur);
			stream.read((char*)&info.vertCount, sizeof uint32_t);

			stream.read((char*)&info.faceCount, sizeof uint32_t);

			stream.seekg(0x10, ios::cur);
			stream.read((char*)&info.indCount, sizeof uint32_t);

			uint32_t compOffset = 0;
			uint32_t bufferOffsetsOff = 0;
			stream.read((char*)&compOffset, sizeof uint32_t);
			stream.read((char*)&bufferOffsetsOff, sizeof uint32_t);

			stream.read((char*)&info.Hash, sizeof uint64_t);

			stream.seekg(0x10, ios::cur);


			uint8_t buffCount = 0;
			stream.read((char*)&buffCount, sizeof buffCount);

			stream.read((char*)&info.indicesStride, sizeof info.indicesStride);

			stream.read((char*)&info.bytesPerVertexOrUnk, sizeof info.bytesPerVertexOrUnk);

			uint8_t compCount = 0;
			stream.read((char*)&compCount, sizeof compCount);

			stream.read((char*)&info.countComp2, sizeof info.countComp2);

			stream.read((char*)&info.flagOrUnk, sizeof info.flagOrUnk);

			stream.read((char*)&info.R32_UNK_Usage, sizeof info.R32_UNK_Usage);

			
			info.Components = vector<Component>(compCount);
			stream.seekg(baseOff + i * sizeof defOff + defOff + compOffset);
			for (uint8_t j = 0; j < compCount; j++)
			{
				Component& comp = info.Components[j];
				stream.read((char*)&comp, sizeof comp);
			}

			
			info.bufferStride = vector<uint16_t>(buffCount);
			for (uint8_t j = 0; j < buffCount; j++)
			{
				uint16_t& stride = info.bufferStride[j];
				stride = 0;
				for (uint8_t f = 0; f < compCount; f++)
				{
					if (j == info.Components[f].bufferIndex)
					{
						switch (info.Components[f].dataType)
						{
						case DataTypes::R32_FLOAT:
						case DataTypes::R32_UNKNOWN:
						case DataTypes::R10G10B10A2_TYPELESS:
							stride += 4 * info.Components[f].elementCount;
							break;
						case DataTypes::R16_UNKNOWN:
						case DataTypes::R16_UINT:
						case DataTypes::R16_SNORM:
						case DataTypes::R16_UNORM:
							stride += 2 * info.Components[f].elementCount;
							break;
						case DataTypes::R8_UINT:
							stride += 1 * info.Components[f].elementCount;
							break;
						case DataTypes::R8_UNKNOWN:
							stride += 1 * info.Components[f].elementCount;
							break;
						default:
							throw std::exception("invalid struct");
							break;
						}
					}
				}
			}

			//for (uint32_t l = 0; l < info.Components.size(); l++)
			//{
			//	dataSet.emplace(info.Components[l].dataType);
			//	compMap[info.Components[l].primitiveType].emplace(std::make_pair(info.Components[l].dataType, info.Components[l].elementCount));
			//}

			info.bufferOffset = vector<uint64_t>(buffCount);
			stream.seekg(baseOff + i * sizeof defOff + defOff + bufferOffsetsOff);
			for (uint8_t j = 0; j < buffCount; j++)
			{
				uint64_t& buffOff = info.bufferOffset[j];
				buffOff = 0;
				stream.read((char*)&buffOff, sizeof uint32_t);
			}

			meshes.push_back(info);
		}

		return true;
	}
	bool MG::Parse(std::iostream& stream, vector<MeshInfo>& meshes, size_t mgBufferSize, size_t baseOff)
	{
		if (mgBufferSize < 0x32)
			return false;
		stream.seekg(0, std::ios::end);
		if (stream.tellg() < mgBufferSize)
			return false;

		uint16_t defCount { 0 };
		stream.seekg(baseOff + 0x30);
		stream.read((char*)&defCount, sizeof defCount);

		uint32_t defOff = 0;

		if (mgBufferSize < (defCount * sizeof defOff + 0x44))
			return false;

		for (uint16_t i = 0; i < defCount; i++)
		{
			stream.seekg(baseOff + i * sizeof defOff + 0x44);

			stream.read((char*)&defOff, sizeof defOff);

			stream.seekg(baseOff + defOff);

			uint16_t parentBone = 0;
			stream.read((char*)&parentBone, sizeof parentBone);

			uint8_t lodCount = 0;
			stream.read((char*)&lodCount, sizeof lodCount);

			uint32_t off = 0;
			for (uint8_t j = 0; j < lodCount; j++)
			{
				stream.seekg(baseOff + defOff + j * sizeof off + 0x38);

				stream.read((char*)&off, sizeof off);

				stream.seekg(baseOff + defOff + off);

				uint32_t cnt = 0;
				stream.read((char*)&cnt, sizeof cnt);

				stream.seekg(0x6, ios::cur);
				for (uint32_t k = 0; k < cnt; k++)
				{
					uint16_t idx = 0;
					stream.read((char*)&idx, sizeof idx);

					meshes[idx].parentBone = parentBone;
					meshes[idx].LODlvl = j;
				}
			}
		}
		return true;
	}
}