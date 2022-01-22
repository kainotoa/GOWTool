#pragma once

#include "pch.h"

class Texpack
{
	uint32_t _texSectionOff {0};
	uint32_t _blocksCount{ 0 };
	uint32_t _blocksInfoOff{ 0 };
	uint32_t _TexsCount{ 0 };

	struct TexInfo
	{
		uint64_t _fileHash;
		uint64_t _userHash;
		uint64_t _blockInfoOff;
	};

	TexInfo* _texInfos {nullptr};
	struct BlockInfo
	{
		uint32_t _blockOff;
		uint32_t _rawSize;
		uint64_t _blockSize;
		uint32_t _unk;
		uint16_t _mipWidth;
		uint16_t _mipHeight;
		uint64_t _nextSiblingBlockInfoOff;
	};
	BlockInfo* _blockInfos{ nullptr };

	uint64_t* _blockInfoOffsets{ nullptr };
	ifstream fs;
public:
	Texpack(const std::filesystem::path &filepath);
	bool ContainsTexture(const uint64_t &hash);
	bool ExportGnf(byte* &output,const uint64_t& hash, uint32_t& expSiz);
	bool ExportGnf(const std::filesystem::path& dir,const uint64_t& hash,std::string name = "");
	bool ExportAllGnf(const std::filesystem::path& dir);
	~Texpack();
};