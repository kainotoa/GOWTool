#include <pch.h>
#include "Formats.h"

void Lodpack::ReadLodpack(std::string filename)
{
	ifstream file(filename, ios::in | ios::binary);
	file.read((char *) &groupCount, sizeof(uint32_t));

	groupStartOff = new uint32_t[groupCount];
	groupHash = new uint64_t[groupCount];
	groupBlockSize = new uint32_t[groupCount];

	file.seekg(16);
	for (uint32_t i = 0; i < groupCount; i++)
	{
		file.read((char *) &groupStartOff[i], sizeof(uint32_t));
		file.seekg(4, file.cur);
	//	std::cout << "\noffset = " << file.tellg() << " index = " << i;
		file.read((char*) &groupHash[i], sizeof(uint64_t));
		file.read((char*) &groupBlockSize[i], sizeof(uint32_t));
		file.seekg(4, file.cur);
	//	std::cout << "\noffset = " << file.tellg() << " index = " << i;
	}

	file.seekg(4);
	file.read((char*) &TotalmembersCount, sizeof(uint32_t));

	memberGroupIndex = new uint32_t[TotalmembersCount];
	memberOffsetter = new uint32_t[TotalmembersCount];
	memberHash = new uint64_t[TotalmembersCount];
	memberBlockSize = new uint32_t[TotalmembersCount];
	
	uint32_t offset = 24 * groupCount + 16;
	file.seekg(offset);
	
	for (uint32_t e = 0; e < TotalmembersCount; e++)
	{
		file.read((char*) &memberGroupIndex[e], sizeof(uint32_t));
		file.read((char*) &memberOffsetter[e], sizeof(uint32_t));
		file.read((char*) &memberHash[e], sizeof(uint64_t));
		file.read((char*) &memberBlockSize[e], sizeof(uint32_t));
		file.seekg(4, std::ios::cur);
	}

	file.close();
}

MGDefinition::~MGDefinition()
{
	delete[] defOffsets;
}

vector<MeshInfo> MGDefinition::ReadMG(std::string filename)
{
	vector<MeshInfo> meshesinfo;
	ifstream file(filename, ios::in | ios::binary);
	file.seekg(56);
	file.read((char*)&defCount, sizeof(uint16_t));

	defOffsets = new uint32_t[defCount];

	file.seekg(76);
	for (uint16_t i = 0; i < defCount; i++)
	{
		file.read((char*)&defOffsets[i], sizeof(uint32_t));
	}

	for (uint16_t i = 0; i < defCount; i++)
	{
		uint32_t offset = defOffsets[i];

		file.seekg((uint64_t)offset);

		uint16_t boneAsso = UINT16_MAX;
		file.read((char*)&boneAsso, sizeof(uint16_t));
		uint8_t subMeshCount = UINT8_MAX;
		file.read((char*)&subMeshCount, sizeof(uint8_t));

		for (uint8_t e = 0; e < subMeshCount; e++)
		{
			uint32_t subMeshOffsetter = UINT32_MAX;
			file.seekg((uint64_t)offset + 60 + (uint64_t)e * 4);
			file.read((char*)&subMeshOffsetter, sizeof(uint32_t));

			uint32_t partCount = UINT32_MAX;
			file.seekg((uint64_t)offset + (uint64_t)subMeshOffsetter);
			file.read((char*)&partCount, sizeof(uint32_t));

			if (partCount == 0)
				continue;

			uint32_t partOffset = (uint32_t)file.tellg() + 8;
			for (uint32_t c = 0; c < partCount; c++)
			{
				MeshInfo info;
				info.LODlvl = e;
				info.boneAssociated = boneAsso;
				file.seekg((uint64_t)partOffset + (uint64_t)c * 4);
				uint32_t off = UINT32_MAX;
				file.read((char*)&off, sizeof(uint32_t));
				off += partOffset + c * 4;

				uint32_t smBaseOff = off;
				file.seekg((uint64_t)smBaseOff + 48);

				uint32_t indOff = UINT32_MAX;
				file.read((char*)&indOff, sizeof(uint32_t));
				info.indicesOffset = indOff;

				file.seekg(4, std::ios::cur);
				uint32_t vertOff = UINT32_MAX;
				file.read((char*)&vertOff, sizeof(uint32_t));
				info.vertexOffset = vertOff;

				file.seekg(4, std::ios::cur);
				uint32_t vertC = UINT32_MAX;
				file.read((char*)&vertC, sizeof(uint32_t));
				info.vertCount = vertC;
				uint32_t indC = UINT32_MAX;
				file.read((char*)&indC, sizeof(uint32_t));
				info.indCount = indC;

				file.seekg((uint64_t)smBaseOff + 16);
				Vec3 extent;
				file.read((char*)&extent.X, sizeof(float));
				file.read((char*)&extent.Y, sizeof(float));
				file.read((char*)&extent.Z, sizeof(float));
				Vec3 origin;
				file.read((char*)&origin.X, sizeof(float));
				file.read((char*)&origin.Y, sizeof(float));
				file.read((char*)&origin.Z, sizeof(float));

				//std::cout << extent.X << " " << extent.Y << " " << extent.Z << " " << origin.X << " " << origin.Y << " " << origin.Z << std::endl;
				Vec3 scale(extent.X * 2, extent.Y * 2, extent.Z * 2);
				Vec3 min(origin.X - extent.X, origin.Y - extent.Y, origin.Z - extent.Z);

				info.meshScale = scale;
				info.meshMin = min;
				file.seekg((uint64_t)smBaseOff + 84);

				uint32_t vertexBlockInfoOffset = UINT32_MAX;
				file.read((char*)&vertexBlockInfoOffset, sizeof(uint32_t));
				uint32_t vertexBlockOffsetterOff = UINT32_MAX;
				file.read((char*)&vertexBlockOffsetterOff, sizeof(uint32_t));

				uint64_t Hash = UINT64_MAX;
				file.read((char*)&Hash, sizeof(uint64_t));
				info.Hash = Hash;

				file.seekg((uint64_t)smBaseOff + (uint64_t)vertexBlockInfoOffset - 8);

				uint8_t buffC = UINT8_MAX;
				file.read((char*)&buffC, sizeof(uint8_t));
				file.seekg(2, std::ios::cur);
				uint8_t compC = UINT8_MAX;
				file.read((char*)&compC, sizeof(uint8_t));

				file.seekg(smBaseOff + vertexBlockOffsetterOff);
				for (uint8_t bIdx = 0; bIdx < buffC; bIdx++)
				{
					uint32_t bufferOff = 0;
					file.read((char*)&bufferOff, sizeof(uint32_t));
					info.bufferOffset.push_back(bufferOff);
				}

				file.seekg((uint64_t)smBaseOff + (uint64_t)vertexBlockInfoOffset);

				for (uint16_t d = 0; d < compC; d++)
				{
					Component component;
					uint8_t val = 0;
					file.read((char*)&val, sizeof(uint8_t));
					component.primitiveType = static_cast<PrimitiveTypes>(val);
					file.read((char*)&val, sizeof(uint8_t));
					component.dataType = static_cast<DataTypes>(val);
					//if(component.primitiveType == PrimitiveTypes::JOINTS0 && (component.dataType != DataTypes::BYTE_STRUCT_0 && component.dataType != DataTypes::HALFWORD_STRUCT_1)/* && component.dataType != DataTypes::HALFWORD_STRUCT_2) */ )
						//cout << file.tellg() << "\n";
					file.read((char*)&component.elementCount, sizeof(uint8_t));
					file.read((char*)&component.offset, sizeof(uint8_t));
					file.read((char*)&component.bufferIndex, sizeof(uint8_t));

					file.seekg(3, std::ios::cur);

					info.Components.push_back(component);
				}
				for (uint8_t bIdx = 0; bIdx < buffC; bIdx++)
				{
					uint16_t stride = 0;
					for (uint8_t f = 0; f < info.Components.size(); f++)
					{
						if (bIdx == info.Components[f].bufferIndex)
						{
							switch (info.Components[f].dataType)
							{
							case DataTypes::FLOAT:
								stride += 4 * info.Components[f].elementCount;
								break;
							case DataTypes::WORD_STRUCT_0:
								stride += 4 * info.Components[f].elementCount;
								break;
							case DataTypes::WORD_STRUCT_1:
								stride += 4 * info.Components[f].elementCount;
								break;
							case DataTypes::HALFWORD_STRUCT_0:
								stride += 2 * info.Components[f].elementCount;
								break;
							case DataTypes::HALFWORD_STRUCT_1:
								stride += 2 * info.Components[f].elementCount;
								break;
							case DataTypes::HALFWORD_STRUCT_2:
								stride += 2 * info.Components[f].elementCount;
								break;
							case DataTypes::UNSIGNED_SHORT:
								stride += 2 * info.Components[f].elementCount;
								break;
							case DataTypes::BYTE_STRUCT_0:
								stride += 1 * info.Components[f].elementCount;
								break;
							default:
								break;
							}
						}
					}
					info.bufferStride.push_back(stride);
				}
				meshesinfo.push_back(info);
			}
		}
	}
	file.close();
	return meshesinfo;
}