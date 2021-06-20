#pragma once

#include <pch.h>

class Texpack
{
	uint32_t _texSectionOff;
	uint32_t _blocksCount;
	uint32_t _blocksInfoOff;
	uint32_t _TexsCount;

	struct BlockInfo
	{
		uint32_t _hash;
		uint32_t _blockOff;
		uint32_t _rawSize;
		uint32_t _blockSize;
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

public:
	Texpack(string);
};