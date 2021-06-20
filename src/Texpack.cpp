#include <pch.h>
#include <../inc/Texpack.h>

Texpack::Texpack(string filename)
{
	ifstream fs = ifstream(filename, ios::in | ios::binary);
	fs.seekg(0, ios::end);
	uint32_t end = fs.tellg();

	fs.seekg(32, ios::beg);

	fs.read((char*)&_texSectionOff, sizeof(uint32_t));
	fs.read((char*)&_blocksCount, sizeof(uint32_t));
	fs.read((char*)&_blocksInfoOff, sizeof(uint32_t));
	fs.read((char*)&_TexsCount, sizeof(uint32_t));

	_blockInfos = new BlockInfo[_blocksCount];

	_blockInfos[0]._blockOff = _texSectionOff;
	fs.seekg(_blocksInfoOff, ios::beg);
	fs.read((char*)&_blockInfos[0]._hash, sizeof(uint32_t));
	fs.read((char*)&_blockInfos[0]._rawSize, sizeof(uint32_t));
	fs.read((char*)&_blockInfos[0]._blockSize, sizeof(uint32_t));
	fs.seekg(8, ios::cur);
	fs.read((char*)&_blockInfos[0]._mipWidth, sizeof(uint16_t));
	fs.read((char*)&_blockInfos[0]._mipHeight, sizeof(uint16_t));
	
	
	fs.seekg(8, ios::cur);
	for (uint32_t i = 1; i < _blocksCount; i++)
	{
		BlockInfo& info = _blockInfos[i];
		info._blockOff = _blockInfos[i - 1]._blockOff + _blockInfos[i - 1]._blockSize;
		fs.read((char*)&info._hash, sizeof(uint32_t));
		fs.read((char*)&info._rawSize, sizeof(uint32_t));
		fs.read((char*)&info._blockSize, sizeof(uint32_t));
		fs.seekg(8, ios::cur);
		fs.read((char*)&info._mipWidth, sizeof(uint16_t));
		fs.read((char*)&info._mipHeight, sizeof(uint16_t));
		fs.seekg(8, ios::cur);
	}

	_texInfos = new TexInfo[_TexsCount];
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		fs.seekg(i*24 + 56, ios::beg);
		TexInfo& info = _texInfos[i];
		fs.read((char*)&info._globHash, sizeof(uint64_t));
		fs.read((char*)&info._locHash, sizeof(uint64_t));
		fs.read((char*)&info._blockInfoOff, sizeof(uint64_t));
		
		uint64_t flag = 0;
		fs.seekg(info._blockInfoOff, ios::beg);
		while (flag != UINT64_MAX)
		{
			uint32_t hash = 0;
			fs.read((char*)&hash, sizeof(uint32_t));
			for (uint32_t i = 0; i < _blocksCount; i++)
			{
				if (hash == _blockInfos[i]._hash)
				{
					info._blocks.push_back(_blockInfos[i]);
					break;
				}
			}
			fs.seekg(20, ios::cur);
			fs.read((char*)&flag, sizeof(uint64_t));
			if (flag != UINT64_MAX)
			{
				fs.seekg(flag, ios::beg);
			}
		}
	}
}