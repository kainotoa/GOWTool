#include "pch.h"
#include "Lodpak.h"

void Lodpack::Read(std::string filename)
{
	file.open(filename, ios::in | ios::binary);
	file.read((char*)&groupCount, sizeof uint32_t);
	file.read((char*)&TotalmembersCount, sizeof uint32_t);

	groupStartOff = new uint64_t[groupCount];
	groupHash = new uint64_t[groupCount];
	groupBlockSize = new uint32_t[groupCount];

	file.seekg(16);
	for (uint32_t i = 0; i < groupCount; i++)
	{
		file.read((char*)&groupStartOff[i], sizeof uint64_t);
		file.read((char*)&groupHash[i], sizeof uint64_t);
		file.read((char*)&groupBlockSize[i], sizeof uint32_t);
		file.seekg(4, std::ios::cur);
	}

	memberGroupIndex = new uint32_t[TotalmembersCount];
	memberOffsetter = new uint32_t[TotalmembersCount];
	memberHash = new uint64_t[TotalmembersCount];
	memberBlockSize = new uint32_t[TotalmembersCount];

	for (uint32_t e = 0; e < TotalmembersCount; e++)
	{
		file.read((char*)&memberGroupIndex[e], sizeof uint32_t);
		file.read((char*)&memberOffsetter[e], sizeof uint32_t);
		file.read((char*)&memberHash[e], sizeof uint64_t);
		file.read((char*)&memberBlockSize[e], sizeof uint32_t);
		file.seekg(4, std::ios::cur);
	}
}
void Lodpack::Write(std::filesystem::path filename, std::map<uint64_t, std::stringstream*>& buffersHashmap)
{
	if (buffersHashmap.size() < 1)
	{
		return;
	}

	file.open(filename, ios::out | ios::binary);

	std::string ss = filename.string() + ".toc";

	std::fstream file1(ss, std::ios::out | std::ios::binary);

	groupCount = buffersHashmap.size();
	TotalmembersCount = buffersHashmap.size();

	size_t writeOff = 0x18 * groupCount * 2 + 0x10;
	writeOff = (writeOff + 15) & ~(15);

	file.write((char*)&groupCount, sizeof groupCount);
	file.write((char*)&TotalmembersCount, sizeof TotalmembersCount);

	file1.write((char*)&groupCount, sizeof groupCount);
	file1.write((char*)&TotalmembersCount, sizeof TotalmembersCount);

	uint64_t temp = 4294967296;
	file.write((char*)&temp, sizeof temp);
	file1.write((char*)&temp, sizeof temp);

	for (auto itr = buffersHashmap.begin(); itr != buffersHashmap.end(); itr++)
	{
		file.write((char*)&writeOff, sizeof writeOff);
		file.write((char*)&itr->first, sizeof itr->first);
		
		file1.write((char*)&writeOff, sizeof writeOff);
		file1.write((char*)&itr->first, sizeof itr->first);

		itr->second->seekg(0, std::ios::end);
		uint32_t size = itr->second->tellg();

		file.write((char*)&size, sizeof size);
		file1.write((char*)&size, sizeof size);


		writeOff += size;
		size = 0;
		file.write((char*)&size, sizeof size);
		file1.write((char*)&size, sizeof size);

	}
	uint32_t idx = 0;
	for (auto itr = buffersHashmap.begin(); itr != buffersHashmap.end(); itr++)
	{
		file.write((char*)&idx, sizeof idx);
		file1.write((char*)&idx, sizeof idx);

		uint32_t size = 0;
		file.write((char*)&size, sizeof size);
		file.write((char*)&itr->first, sizeof itr->first);

		file1.write((char*)&size, sizeof size);
		file1.write((char*)&itr->first, sizeof itr->first);

		itr->second->seekg(0, std::ios::end);
		size = itr->second->tellg();

		file.write((char*)&size, sizeof size);

		file1.write((char*)&size, sizeof size);

		size = 0;
		file.write((char*)&size, sizeof size);
		
		file1.write((char*)&size, sizeof size);

		idx++;
	}

	writeOff = 0x18 * groupCount * 2 + 0x10;
	writeOff = (writeOff + 15) & ~(15);
	while (file.tellp() < writeOff)
	{
		byte b = 0;
		file.write((char*)&b, sizeof b);
	}

	for (auto itr = buffersHashmap.begin(); itr != buffersHashmap.end(); itr++)
	{
		itr->second->seekg(0, std::ios::end);
		size_t size = itr->second->tellg();

		byte* bin = new byte[size];

		itr->second->seekg(0, std::ios::beg);
		itr->second->read((char*)bin, size);

		file.write((char*)bin, size);

		delete[] bin;
	}

	file.close();
	file1.close();
}
bool Lodpack::GetBuffer(const uint64_t& Hash, std::stringstream& outstream)
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
bool Lodpack::SetBuffer(const uint64_t& Hash, std::iostream& stream)
{
	for (uint32_t e = 0; e < TotalmembersCount; e++)
	{
		if (memberHash[e] == Hash)
		{
			std::unique_ptr<char[]> arr = std::make_unique<char[]>(memberBlockSize[e]);
			stream.seekg(0, std::ios::beg);
			stream.read(arr.get(), memberBlockSize[e]);
			file.seekp(memberOffsetter[e] + groupStartOff[memberGroupIndex[e]], std::ios::beg);
			file.write(arr.get(), memberBlockSize[e]);
			return true;
		}
	}
	return false;
}