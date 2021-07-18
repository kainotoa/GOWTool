#include "pch.h"
#include "Lodpak.h"

Lodpack::Lodpack(std::string filename)
{
	file.open(filename, ios::in | ios::binary);
	file.read((char*)&groupCount, sizeof(uint32_t));

	groupStartOff = new uint32_t[groupCount];
	groupHash = new uint64_t[groupCount];
	groupBlockSize = new uint32_t[groupCount];

	file.seekg(16);
	for (uint32_t i = 0; i < groupCount; i++)
	{
		file.read((char*)&groupStartOff[i], sizeof(uint32_t));
		file.seekg(4, file.cur);
		file.read((char*)&groupHash[i], sizeof(uint64_t));
		file.read((char*)&groupBlockSize[i], sizeof(uint32_t));
		file.seekg(4, file.cur);
	}

	file.seekg(4);
	file.read((char*)&TotalmembersCount, sizeof(uint32_t));

	memberGroupIndex = new uint32_t[TotalmembersCount];
	memberOffsetter = new uint32_t[TotalmembersCount];
	memberHash = new uint64_t[TotalmembersCount];
	memberBlockSize = new uint32_t[TotalmembersCount];

	uint32_t offset = 24 * groupCount + 16;
	file.seekg(offset);

	for (uint32_t e = 0; e < TotalmembersCount; e++)
	{
		file.read((char*)&memberGroupIndex[e], sizeof(uint32_t));
		file.read((char*)&memberOffsetter[e], sizeof(uint32_t));
		file.read((char*)&memberHash[e], sizeof(uint64_t));
		file.read((char*)&memberBlockSize[e], sizeof(uint32_t));
		file.seekg(4, std::ios::cur);
	}
}
bool Lodpack::GetBuffer(uint64_t& Hash, std::stringstream& outstream)
{
	outstream.str("");
	outstream.clear();
	for (uint32_t e = 0; e < TotalmembersCount; e++)
	{
		if (memberHash[e] == Hash)
		{
			std::unique_ptr<char[]> arr = std::make_unique<char[]>(memberBlockSize[e]);
			file.seekg(memberOffsetter[e] + groupStartOff[memberGroupIndex[e]], std::ios::beg);
			file.read(arr.get(), memberBlockSize[e]);
			outstream.write(arr.get(), memberBlockSize[e]);
			return true;
		}
	}
	return false;
}