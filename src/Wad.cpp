#include "pch.h"
#include "Wad.h"

WADFile::WADFile(string filename)
{
	fs.open(filename, ios::in | ios::binary);
	fs.seekg(0, ios::end);
	size_t end = fs.tellg();

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
bool WADFile::GetBuffer(uint32_t entryIdx,std::stringstream& outstream)
{
	outstream.str("");
	outstream.clear();
	if (entryIdx >= _Entries.size())
		return false;
	std::unique_ptr<char[]> arr = std::make_unique<char[]>(_Sizes[entryIdx]);
	fs.seekg(_Offsets[entryIdx], std::ios::beg);
	fs.read(arr.get(), _Sizes[entryIdx]);
	outstream.write(arr.get(), _Sizes[entryIdx]);
	return true;
}