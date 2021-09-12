#pragma once

#include <pch.h>
typedef unsigned char byte;

class Texpack
{
	uint32_t _texSectionOff;
	uint32_t _blocksCount;
	uint32_t _blocksInfoOff;
	uint32_t _TexsCount;

	struct BlockInfo
	{
		uint32_t _hash;
		uint64_t _blockOff;
		uint32_t _rawSize;
		uint64_t _blockSize;
		uint16_t _mipWidth;
		uint16_t _mipHeight;
	};
	BlockInfo* _blockInfos;
	struct TexInfo
	{
		uint64_t _globHash;
		uint64_t _locHash;
		uint64_t _blockInfoOff;
		vector<BlockInfo> _blocks;
	};
	TexInfo* _texInfos;

	ifstream fs;
public:
	Texpack(string);
	bool ContainsTexture(uint64_t);
	void ExportTexture(byte* output, uint64_t hash, uint32_t& expSiz);
	void ExportAll(string dir);
	~Texpack();
};