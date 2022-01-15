#include "pch.h"
#include "Rig.h"

Rig::Rig(std::stringstream& fs)
{
	fs.seekg(12, std::ios::beg);
	fs.read((char*)&boneCount, sizeof(uint16_t));
	boneParents = new int16_t[boneCount];
	for (uint16_t i = 0; i < boneCount; i++)
	{
		fs.seekg(0x2E + 8 * i,std::ios::beg);
		fs.read((char*)&boneParents[i],sizeof(int16_t));
	}

	boneNames = new string[boneCount];
	uint32_t boneNamesOffset = 0;
	fs.seekg(0x18, std::ios::beg);
	fs.read((char*)&boneNamesOffset, sizeof(uint32_t));
	boneNamesOffset -= boneCount * 0x38;
	char name[56];
	for (uint16_t i = 0; i < boneCount; i++)
	{
		fs.seekg(boneNamesOffset + 56 * i, std::ios::beg);
		fs.read(name, sizeof(name));
		boneNames[i] = name;
	}

	matrix = new Matrix4x4[boneCount];

	uint32_t matricesOffset;
	fs.seekg(0x1C, std::ios::beg);
	fs.read((char*)&matricesOffset, sizeof(uint32_t));
	matricesOffset += 0x50;

	for (uint16_t i = 0; i < boneCount; i++)
	{
		fs.seekg(matricesOffset + i * 64, std::ios::beg);

		for (size_t r = 0; r < 4; r++)
		{
			for (size_t c = 0; c < 4; c++)
			{
				fs.read((char*)&matrix[i][r][c], sizeof(float));
			}
		}
	}

	IBMs = new Matrix4x4[boneCount];
	uint32_t IBMsOffset = matricesOffset + (boneCount) * 64;
	for (uint16_t i = 0; i < boneCount; i++)
	{
		fs.seekg(IBMsOffset + i * 64, std::ios::beg);

		for (size_t r = 0; r < 4; r++)
		{
			for (size_t c = 0; c < 4; c++)
			{
				fs.read((char*)&IBMs[i][r][c], sizeof(float));
			}
		}
	}
}