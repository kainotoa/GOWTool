#include <pch.h>

#include "Wad.h"

WADFile::WADFile(string filename)
{
	ifstream fs;
	fs.open(filename, ios::in | ios::binary);

	fs.seekg(0, ios::end);
	uint32_t end = fs.tellg();

	uint16_t group = 0;
	uint32_t size = 0;
	char name[0x38];

	uint64_t pad = 0;
	fs.seekg(0, ios::beg);
	while (fs.tellg() < end)
	{
		fs.read((char*)&group, sizeof(uint16_t));
		fs.seekg(2, ios::cur);
		fs.read((char*)&size, sizeof(uint32_t));
		fs.seekg(0x10, ios::cur);
		int a = fs.tellg();
		fs.read(name, sizeof(name));
		fs.seekg(0x10, ios::cur);
		a = fs.tellg();
		if (size != 0)
		{
			_Entries.push_back(name);
			_Offsets.push_back(static_cast<uint32_t>(fs.tellg()));
			_Sizes.push_back(size);
			_Groups.push_back(group);

			fs.seekg(size, ios::cur);
			pad = fs.tellg();
			if (pad % 16 != 0)
			{
				fs.seekg((pad / 16 + 1) * 16, ios::beg);
			}
			a = fs.tellg();
		}
	}

}