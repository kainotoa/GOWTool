#include <pch.h>
#include "Formats.h"

MGDefinition::~MGDefinition()
{
	delete[] defOffsets;
}

vector<MeshInfo> MGDefinition::ReadMG(std::stringstream& file)
{
	vector<MeshInfo> meshesinfo;
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
					if (component.primitiveType == PrimitiveTypes::UNKNOWN0)/* && (component.dataType != DataTypes::BYTE_STRUCT_0 && component.dataType != DataTypes::HALFWORD_STRUCT_1) && component.dataType != DataTypes::HALFWORD_STRUCT_2) */
						cout << file.tellg() << "\n";
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
	return meshesinfo;
}
vector<MeshInfo> SmshDefinition::ReadSmsh(std::iostream& file)
{
	uint32_t groupDataOffset = 0;
	uint32_t meshDefSectionOff = 0;
	uint32_t meshDefCnt = 0;
	file.seekg(0x48, ios::beg);
	file.read((char*)&groupDataOffset, sizeof(uint32_t));
	groupDataOffset += 0x48;
	file.seekg(0x58, ios::beg);
	file.read((char*)&meshDefSectionOff, sizeof(uint32_t));
	file.read((char*)&meshDefCnt, sizeof(uint32_t));
	meshDefSectionOff += 0x58;


	uint64_t currHash = UINT64_MAX;
	uint16_t curGroup = UINT16_MAX;
	uint16_t curLod = 0;
	vector<MeshInfo> meshinfos;
	for (uint32_t i = 0; i < meshDefCnt; i++)
	{
		file.seekg(meshDefSectionOff + i * 4, ios::beg);
		uint32_t off = 0;
		file.read((char*)&off, sizeof(uint32_t));
		off += i * 4;
		file.seekg(meshDefSectionOff + off, ios::beg);

		file.seekg(0x10, ios::cur);
		Vec3 extent;
		file.read((char*)&extent.X, sizeof(float));
		file.read((char*)&extent.Y, sizeof(float));
		file.read((char*)&extent.Z, sizeof(float));
		Vec3 origin;
		file.read((char*)&origin.X, sizeof(float));
		file.read((char*)&origin.Y, sizeof(float));
		file.read((char*)&origin.Z, sizeof(float));

		Vec3 scale(extent.X * 2, extent.Y * 2, extent.Z * 2);
		Vec3 min(origin.X - extent.X, origin.Y - extent.Y, origin.Z - extent.Z);

		MeshInfo info;
		info.meshScale = scale;
		info.meshMin = min;
		info.boneAssociated = 0;

		file.seekg(meshDefSectionOff + off + 0x30, ios::beg);
		uint32_t temp = 0;
		file.read((char*)&temp, sizeof(uint32_t));
		info.indicesOffset = temp;
		file.seekg(4, std::ios::cur);
		file.read((char*)&temp, sizeof(uint32_t));
		info.vertexOffset = temp;
		file.seekg(4, std::ios::cur);
		file.read((char*)&info.vertCount, sizeof(uint32_t));
		file.read((char*)&info.indCount, sizeof(uint32_t));

		file.seekg(meshDefSectionOff + off + 0x54, ios::beg);
		uint32_t vertexInfoOffset = 0;
		uint32_t vertexDataOffsOffset = 0;
		file.read((char*)&vertexInfoOffset, sizeof(uint32_t));
		file.read((char*)&vertexDataOffsOffset, sizeof(uint32_t));
		file.read((char*)&info.Hash, sizeof(uint64_t));

		file.seekg(meshDefSectionOff + off + vertexInfoOffset - 8);

		uint8_t buffC = 0;
		file.read((char*)&buffC, sizeof(uint8_t));
		file.seekg(2, std::ios::cur);
		uint8_t compC = 0;
		file.read((char*)&compC, sizeof(uint8_t));

		file.seekg(meshDefSectionOff + off + vertexInfoOffset);
		for (uint16_t d = 0; d < compC; d++)
		{
			Component component;
			uint8_t val = 0;
			file.read((char*)&val, sizeof(uint8_t));
			component.primitiveType = static_cast<PrimitiveTypes>(val);
			file.read((char*)&val, sizeof(uint8_t));
			component.dataType = static_cast<DataTypes>(val);
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

		file.seekg(meshDefSectionOff + off + vertexDataOffsOffset);
		for (uint8_t bIdx = 0; bIdx < buffC; bIdx++)
		{
			uint32_t bufferOff = 0;
			file.read((char*)&bufferOff, sizeof(uint32_t));
			info.bufferOffset.push_back(bufferOff);
		}

		file.seekg(groupDataOffset + i * 2, ios::beg);
		uint16_t group = 0;
		file.read((char*)&group, sizeof(uint16_t));
		if (curGroup == group && currHash != info.Hash)
		{
			++curLod;
		}
		if (curGroup != group)
		{
			curGroup = group;
			curLod = 0;
		}
		info.LODlvl = curLod;
		if (currHash != info.Hash)
		{
			currHash = info.Hash;
		}

		meshinfos.push_back(info);
	}
	return meshinfos;
}
bool MGDefinition::WriteMG(const std::vector<MeshInfo>& meshinfos, std::stringstream& file)
{
	file.seekg(56);
	file.read((char*)&defCount, sizeof(uint16_t));

	defOffsets = new uint32_t[defCount];

	file.seekg(76);
	for (uint16_t i = 0; i < defCount; i++)
	{
		file.read((char*)&defOffsets[i], sizeof(uint32_t));
	}

	int meshCnt = 0; 
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

				char buf[10];
				sprintf_s(buf, "%04d", meshCnt);
				string subname = "submesh_" + string(buf) + "_" + std::to_string(info.LODlvl);
				bool yes = false;
				MeshInfo newinfo;
				meshCnt++;
				for (int ppza = 0; ppza < meshinfos.size(); ppza++)
				{
					if (subname == meshinfos[ppza].name)
					{
						yes = true;
						newinfo = meshinfos[ppza];
					}
				}
				file.seekg((uint64_t)partOffset + (uint64_t)c * 4);
				uint32_t off = UINT32_MAX;
				file.read((char*)&off, sizeof(uint32_t));
				off += partOffset + c * 4;

				uint32_t smBaseOff = off;
				file.seekg((uint64_t)smBaseOff + 48);

				uint32_t indOff = UINT32_MAX;
				file.seekp(file.tellg());
				file.read((char*)&indOff, sizeof(uint32_t));

				info.indicesOffset = indOff;
				indOff = newinfo.indicesOffset;
				if(yes)
					file.write((char*)&indOff, sizeof(uint32_t));

				file.seekg(4, std::ios::cur);
				file.seekp(file.tellg());

				uint32_t vertOff = UINT32_MAX;
				file.read((char*)&vertOff, sizeof(uint32_t));
				info.vertexOffset = vertOff;
				vertOff = newinfo.vertexOffset;
				if (yes)
					file.write((char*)&vertOff, sizeof(uint32_t));

				file.seekg(4, std::ios::cur);

				file.seekp(file.tellg());

				uint32_t vertC = UINT32_MAX;
				file.read((char*)&vertC, sizeof(uint32_t));
				info.vertCount = vertC;

				vertC = newinfo.vertCount;
				if (yes)
					file.write((char*)&vertC, sizeof(uint32_t));

				uint32_t indC = UINT32_MAX;
				file.read((char*)&indC, sizeof(uint32_t));
				info.indCount = indC;
				indC = newinfo.indCount;
				if (yes)
					file.write((char*)&indC, sizeof(uint32_t));

				file.seekg((uint64_t)smBaseOff + 16);

				file.seekp(file.tellg());

				Vec3 extent;
				file.read((char*)&extent.X, sizeof(float));
				file.read((char*)&extent.Y, sizeof(float));
				file.read((char*)&extent.Z, sizeof(float));
				Vec3 origin;
				file.read((char*)&origin.X, sizeof(float));
				file.read((char*)&origin.Y, sizeof(float));
				file.read((char*)&origin.Z, sizeof(float));

				extent = Vec3(newinfo.meshScale.X * 2, newinfo.meshScale.Y * 2, newinfo.meshScale.Z * 2);
				origin = Vec3(newinfo.meshMin.X + extent.X, newinfo.meshMin.Y + extent.Y, newinfo.meshMin.Z + extent.Z);

				if (yes)
				{
					file.write((char*)&extent.X, sizeof(float));
					file.write((char*)&extent.Y, sizeof(float));
					file.write((char*)&extent.Z, sizeof(float));
					file.write((char*)&origin.X, sizeof(float));
					file.write((char*)&origin.Y, sizeof(float));
					file.write((char*)&origin.Z, sizeof(float));
				}

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
				file.seekp(file.tellg());
				for (uint8_t bIdx = 0; bIdx < buffC; bIdx++)
				{
					uint32_t bufferOff = 0;
					file.read((char*)&bufferOff, sizeof(uint32_t));
					info.bufferOffset.push_back(bufferOff);
					if(yes)
						file.write((char*)&newinfo.bufferOffset[bIdx], sizeof(uint32_t));
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
					if (component.primitiveType == PrimitiveTypes::UNKNOWN0)/* && (component.dataType != DataTypes::BYTE_STRUCT_0 && component.dataType != DataTypes::HALFWORD_STRUCT_1) && component.dataType != DataTypes::HALFWORD_STRUCT_2) */
						cout << file.tellg() << "\n";
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
			}
		}
	}
	return true;
}