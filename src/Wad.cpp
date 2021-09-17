#include "pch.h"
#include "Wad.h"

bool WadFile::Read(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
		return false;

	fs.open(filepath.string(), ios::in | ios::binary);
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
		fs.read(name, sizeof(name));
		fs.seekg(0x10, ios::cur);
		if (size != 0)
		{
			_Entries.push_back(name);
			_Offsets.push_back(static_cast<uint32_t>(fs.tellg()));
			_Sizes.push_back(size);
			_Groups.push_back(group);

			fs.seekg(size, ios::cur);
			pad = fs.tellg();
			pad = (pad + 15) & (~15);
			fs.seekg(pad, ios::beg);
		}
	}
	return true;
}
bool WadFile::GetBuffer(const uint32_t& entryIdx,uint8_t* output)
{
	if (entryIdx >= _Entries.size())
		return false;
	output = new uint8_t[_Sizes[entryIdx]];
	fs.seekg(_Offsets[entryIdx], std::ios::beg);
	fs.read((char*)output, _Sizes[entryIdx]);
	return true;
}
bool WadFile::GetBuffer(const uint32_t& entryIdx, std::iostream& outstream)
{
	if (entryIdx >= _Entries.size())
		return false;
	std::unique_ptr<uint8_t[]> output = std::make_unique<uint8_t[]>(_Sizes[entryIdx]);
	fs.seekg(_Offsets[entryIdx], std::ios::beg);
	fs.read((char*)output.get(), _Sizes[entryIdx]);
	outstream.write((char*)output.get(), _Sizes[entryIdx]);
	return true;
}