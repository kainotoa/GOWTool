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
void WadFile::WriteBufferToWad(std::iostream& outWadStream, const byte* buffer, const size_t& bufferSize, const size_t& bufferIdx)
{
    size_t cnt = 0;
    outWadStream.seekg(0, ios::end);
    size_t end = outWadStream.tellg();

    uint64_t pad = 0;
    outWadStream.seekg(0, ios::beg);
    while (outWadStream.tellg() < end)
    {
        WadFile::FileDesc entry;
        outWadStream.read((char*)&entry.group, sizeof(uint16_t));
        outWadStream.read((char*)&entry.type, sizeof(uint16_t));
        size_t off = outWadStream.tellg();
        outWadStream.read((char*)&entry.size, sizeof(uint32_t));
        if (cnt == bufferIdx)
        {
            outWadStream.seekp(off);
            uint32_t tempSize = uint32_t(bufferSize);
            outWadStream.write((char*)&tempSize, sizeof(tempSize));
        }
        outWadStream.seekg(0x10, ios::cur);
        char name[0x38];
        outWadStream.read(name, sizeof(name));
        entry.name = string(name);
        outWadStream.seekg(0x10, ios::cur);
        if (entry.size != 0)
        {
            entry.offset = static_cast<uint32_t>(outWadStream.tellg());
            outWadStream.seekg(entry.size, ios::cur);
            pad = outWadStream.tellg();
            pad = (pad + 15) & (~15);
            outWadStream.seekg(pad, ios::beg);
            if (cnt == bufferIdx)
            {
                size_t forwardSize = end - pad;
                byte* forwardBytes = new byte[forwardSize];
                outWadStream.read((char*)forwardBytes, forwardSize);
                outWadStream.seekp(entry.offset);
                outWadStream.write((char*)buffer, bufferSize);
                size_t padSize = ((bufferSize + 15) & (~15)) - bufferSize;
                if (padSize > 0)
                {
                    byte* paddingBytes = new byte[padSize];
                    std::fill(paddingBytes, paddingBytes + padSize, 0);
                    outWadStream.write((char*)paddingBytes, padSize);
                    delete[] paddingBytes;
                }
                outWadStream.write((char*)forwardBytes, forwardSize);
                end = outWadStream.tellp();
                outWadStream.seekg(entry.offset + bufferSize + padSize, std::ios::beg);
                delete[] forwardBytes;
            }
            cnt++;
        }
    }
}