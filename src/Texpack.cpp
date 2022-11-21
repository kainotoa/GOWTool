#include "pch.h"
#include "Texpack.h"
#include "krak.h"
#include "Gnf.h"
#include "converter.h"
#include "utils.h"

Texpack::Texpack(const std::filesystem::path& filepath)
{
	fs = ifstream(filepath.string(), ios::in | ios::binary);
	fs.seekg(0, ios::end);
	size_t end = fs.tellg();

	fs.seekg(0x20, ios::beg);

	fs.read((char*)&_texSectionOff, sizeof(uint32_t));
	fs.read((char*)&_blocksCount, sizeof(uint32_t));
	fs.read((char*)&_blocksInfoOff, sizeof(uint32_t));
	fs.read((char*)&_TexsCount, sizeof(uint32_t));

	_texInfos = new TexInfo[_TexsCount];

	fs.seekg(0x38, ios::beg);
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		TexInfo& info = _texInfos[i];
		fs.read((char*)&info, sizeof(info));
	}

	_blockInfos = new BlockInfo[_blocksCount];
	_blockInfoOffsets = new uint64_t[_blocksCount];
	fs.seekg(_blocksInfoOff, ios::beg);
	for (uint32_t i = 0; i < _blocksCount; i++)
	{
		_blockInfoOffsets[i] = fs.tellg();
		BlockInfo& info = _blockInfos[i];
		fs.read((char*)&info, sizeof(info));
	}
}
Texpack::~Texpack()
{
	fs.close();
	delete[] _blockInfos;
	delete[] _texInfos;
}
bool Texpack::ContainsTexture(const uint64_t& hash)
{
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		if (_texInfos[i]._fileHash == hash)
		{
			return true;
		}
	}
	return false;
}
bool Texpack::ExportGnf(byte*& output, const uint64_t& hash, uint32_t& expSize)
{
	if (!OodLZ_Decompress)
		return false;

	TexInfo* texInfo = nullptr;
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		if (_texInfos[i]._fileHash == hash)
		{
			texInfo = &_texInfos[i];
			break;
		}
	}
	if (texInfo == nullptr)
		return false;

	std::vector<BlockInfo*> texblockInfos;

	for (uint32_t i = 0; i < _blocksCount; i++)
	{
		if (texInfo->_blockInfoOff == _blockInfoOffsets[i])
		{
			texblockInfos.push_back(&_blockInfos[i]);
			break;
		}
	}
	if (texblockInfos.size() < 1)
		return false;
	while (texblockInfos.front()->_nextSiblingBlockInfoOff != -1LL)
	{
		for (uint32_t i = 0; i < _blocksCount; i++)
		{
			if (texblockInfos.front()->_nextSiblingBlockInfoOff == _blockInfoOffsets[i])
			{
				texblockInfos.insert(texblockInfos.begin(), &_blockInfos[i]);
				break;
			}
		}
	}
	uint32_t writeSize = 0x100; //gnf Header
	for (uint32_t i = 0; i < texblockInfos.size(); i++)
	{
		writeSize += texblockInfos[i]->_rawSize;
	}
	output = new byte[writeSize];

	uint32_t writeOff = 0;
	for (uint32_t i = 0; i < texblockInfos.size(); i++)
	{
		fs.seekg((size_t(texblockInfos[i]->_blockOff) << 4) + 4, std::ios::beg);
		uint32_t off = 0;
		uint32_t len = 0;
		fs.read((char*)&off, sizeof(uint32_t));
		fs.read((char*)&len, sizeof(uint32_t));
		fs.seekg(4, std::ios::cur);
		if (off != 0x1CU)
		{
			fs.read((char*)(output + writeOff), 0x100);
			writeOff += 0x100;
			fs.seekg(4, std::ios::cur);
		}
		uint32_t decSize = 0;
		fs.read((char*)&decSize, sizeof(uint32_t));
		fs.seekg(8, std::ios::cur);
		byte* readbytes = new byte[len - off];
		byte* writebytes = new byte[decSize];
		fs.read((char*)readbytes, (len - off));
		uint32_t status = OodLZ_Decompress(readbytes, len - off, writebytes, decSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if (status != decSize)
		{
			throw std::exception("Decompression Failed!");
		}
		memmove(output + writeOff, writebytes, decSize);
		delete[] readbytes;
		delete[] writebytes;
		writeOff += decSize;
	}
	expSize = writeSize;
	return true;
}
bool Texpack::ExportGnf(const std::filesystem::path& dir, const uint64_t& hash, std::string name,bool dds)
{
	if (!std::filesystem::exists(dir))
		return false;
	byte* output = nullptr;
	uint32_t size = 0;
	bool result = ExportGnf(output, hash, size);
	if (!result)
		return false;

	std::filesystem::path outpath = dir;
	if (name.empty())
	{
		std::stringstream s;
		s << std::hex << hash;
		outpath /= (Utils::str_toupper(s.str()) + ".gnf");
	}
	else
	{
		outpath /= (name + ".gnf");
	}

	if (dds)
	{
		outpath.replace_extension(std::filesystem::path(".dds"));

		byte* ddsout = nullptr;
		size = ConvertGnfToDDS(output,size,ddsout);
		std::ofstream ofs(outpath.string(), ios::binary | ios::out);
		ofs.write((char*)ddsout, size);
		ofs.close();
		delete[] ddsout;
	}
	else
	{
		std::ofstream ofs(outpath.string(), ios::binary | ios::out);
		ofs.write((char*)output, size);
		ofs.close();
	}

	delete[] output;
	return true;
}
bool Texpack::ExportAllGnf(const std::filesystem::path& dir,bool dds)
{
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		ExportGnf(dir, _texInfos[i]._fileHash,"",dds);
	}
	return true;
}
bool Texpack::GetUserHash(const uint64_t& hash, uint64_t& outUserHash)
{
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		if (_texInfos[i]._fileHash == hash)
		{
			outUserHash = _texInfos[i]._userHash;
			return true;
		}
	}
	return false;
}

