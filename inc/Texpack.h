#pragma once

#include "pch.h"

class Texpack
{
	uint32_t _texSectionOff {0};
	uint32_t _blocksCount{ 0 };
	uint32_t _blocksInfoOff{ 0 };
	uint32_t _TexsCount{ 0 };
public:
	struct TexInfo
	{
		uint64_t _fileHash;
		uint64_t _userHash;
		uint64_t _blockInfoOff;
	};

	struct BlockInfo
	{
		uint32_t _blockOff;
		uint32_t _rawSize;
		uint64_t _blockSize;
		byte _mipLvlStart;
		byte _mipLvlEnd;
		uint16_t _tocFileIdx;
		uint16_t _mipWidth;
		uint16_t _mipHeight;
		uint64_t _nextSiblingBlockInfoOff;
	};
private:
	TexInfo* _texInfos{ nullptr };
	BlockInfo* _blockInfos{ nullptr };

	uint64_t* _blockInfoOffsets{ nullptr };
	ifstream fs;
public:
	Texpack(const std::filesystem::path &filepath);
	bool ContainsTexture(const uint64_t &hash);
	bool ExportGnf(byte* &output,const uint64_t& hash, uint32_t& expSiz);
	bool ExportGnf(const std::filesystem::path& dir,const uint64_t& hash,std::string name = "",bool dds = false);
	bool ExportAllGnf(const std::filesystem::path& dir,bool dds = false);
	bool GetUserHash(const uint64_t& hash, uint64_t& outUserHash);
	~Texpack();
};