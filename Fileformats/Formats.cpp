#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>

class Lodpack
{
	
public:
	uint32_t groupCount;
	uint32_t* groupStartOff;
	uint64_t* groupHash;
	uint32_t* groupBlockSize;

	
	uint32_t TotalmembersCount;
	uint32_t* memberGroupIndex;
	uint32_t* memberOffsetter;
	uint64_t* memberHash;
	uint32_t* memberBlockSize;
	
	void ReadLodpack(std::string filename)
	{
		std::ifstream file;

		file.open(filename, std::ios::in | std::ios::binary);
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
	}
};