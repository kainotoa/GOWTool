#include "pch.h"
#include "Wad.h"

bool WadFile::Read(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
		return false;

	fs.open(filepath.string(), ios::in | ios::binary);
	fs.seekg(0, ios::end);
	size_t end = fs.tellg();

	uint64_t pad = 0;
	fs.seekg(0, ios::beg);
	while (fs.tellg() < end)
	{
		FileDesc entry;
		fs.read((char*)&entry.group, sizeof(uint16_t));
		fs.read((char*)&entry.type, sizeof(uint16_t));
		fs.read((char*)&entry.size, sizeof(uint32_t));
		fs.seekg(0x10, ios::cur);
		char name[0x38];
		fs.read(name, sizeof(name));
		entry.name = string(name);
		fs.seekg(0x10, ios::cur);
		if (entry.size != 0)
		{
			entry.offset = static_cast<uint32_t>(fs.tellg());
			_FileEntries.push_back(entry);
			fs.seekg(entry.size, ios::cur);
			pad = fs.tellg();
			pad = (pad + 15) & (~15);
			fs.seekg(pad, ios::beg);
		}
	}
	return true;
}
bool WadFile::GetBuffer(const uint32_t& entryIdx,uint8_t* output)
{
	
	if (entryIdx >= _FileEntries.size())
		return false;
	output = new uint8_t[_FileEntries[entryIdx].size];
	fs.seekg(_FileEntries[entryIdx].offset, std::ios::beg);
	fs.read((char*)output, _FileEntries[entryIdx].size);
	
	return true;
	
}
bool WadFile::GetBuffer(const uint32_t& entryIdx, std::iostream& outstream)
{
	
	if (entryIdx >= _FileEntries.size())
		return false;
	std::unique_ptr<uint8_t[]> output = std::make_unique<uint8_t[]>(_FileEntries[entryIdx].size);
	fs.seekg(_FileEntries[entryIdx].offset, std::ios::beg);
	fs.read((char*)output.get(), _FileEntries[entryIdx].size);
	outstream.write((char*)output.get(), _FileEntries[entryIdx].size);
	
	return true;
}