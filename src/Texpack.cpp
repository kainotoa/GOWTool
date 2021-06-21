#include <pch.h>
#include <../inc/Texpack.h>
#include <../inc/krak.h>

Texpack::Texpack(string filename)
{
	fs = ifstream(filename, ios::in | ios::binary);
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
	fs.read((char*)&_blockInfos[0]._blockSize, sizeof(uint64_t));
	fs.seekg(4, ios::cur);
	fs.read((char*)&_blockInfos[0]._mipWidth, sizeof(uint16_t));
	fs.read((char*)&_blockInfos[0]._mipHeight, sizeof(uint16_t));
	
	
	fs.seekg(8, ios::cur);
	for (uint32_t i = 1; i < _blocksCount; i++)
	{
		BlockInfo& info = _blockInfos[i];
		info._blockOff = _blockInfos[i - 1]._blockOff + _blockInfos[i - 1]._blockSize;
		fs.read((char*)&info._hash, sizeof(uint32_t));
		fs.read((char*)&info._rawSize, sizeof(uint32_t));
		fs.read((char*)&info._blockSize, sizeof(uint64_t));
		fs.seekg(4, ios::cur);
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
	LoadLib();
}
Texpack::~Texpack()
{
	fs.close();
	delete[] _blockInfos;
	delete[] _texInfos;
}
bool Texpack::ContainsTexture(uint64_t hash)
{
	for (int i = 0; i < _TexsCount; i++)
	{
		if (_texInfos[i]._globHash == hash)
		{
			return true;
		}
	}
	return false;
}
byte* Texpack::ExportTexture(uint64_t hash, uint32_t& expSize)
{
	uint32_t writeSize = 0x100;			// for gnf header
	uint32_t writeOff = 0;
	uint32_t texIdx = 0;
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		if (_texInfos[i]._globHash == hash)
		{
			texIdx = i;
			for (int e = 0; e < _texInfos[i]._blocks.size(); e++)
			{
				writeSize += _texInfos[i]._blocks[e]._rawSize;
			}
			break;
		}
	}

	byte* output = new byte[writeSize];

	uint64_t offset = 0;
	uint32_t last = _texInfos[texIdx]._blocks.size() - 1;

	offset = _texInfos[texIdx]._blocks[last]._blockOff;
	byte* gnf = new byte[0x100];
	fs.seekg(offset + 0x10, ios::beg);
	fs.read((char*)gnf, 0x100);

	memmove(output + writeOff, gnf, 0x100);
	writeOff += 0x100;

	for (int e = _texInfos[texIdx]._blocks.size() - 1; e >= 0; e--)
	{
		offset = _texInfos[texIdx]._blocks[e]._blockOff;

		uint64_t rawSize = _texInfos[texIdx]._blocks[e]._rawSize;
		byte* outbytes = new byte[rawSize + SAFE_SPACE];

		uint32_t off = 0;
		fs.seekg(offset + 4, ios::beg);
		fs.read((char*)&off, sizeof(uint32_t));

		uint32_t Size = _texInfos[texIdx]._blocks[e]._blockSize - off;

		byte* inbytes = new byte[Size];
		fs.seekg(offset + off, ios::beg);
		fs.read((char*)inbytes, Size);

		uint32_t status = OodLZ_Decompress(inbytes, Size, outbytes, rawSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		/*
		if (status == rawSize)
			cout << " Good";
		else
			cout << " bad";
		*/
		memmove(output + writeOff, outbytes, rawSize);
		writeOff += rawSize;
		delete[] inbytes;
		delete[] outbytes;
	}
	delete[] gnf;
	expSize = writeSize;

	return output;
}
void Texpack::ExportAll(string dir)
{
	uint32_t texIdx = 0;
	for (uint32_t i = 0; i < _TexsCount; i++)
	{
		texIdx = i;
		uint32_t writeSize = 0x100;			// for gnf header
		uint32_t writeOff = 0;
		for (int e = 0; e < _texInfos[texIdx]._blocks.size(); e++)
		{
			writeSize += _texInfos[texIdx]._blocks[e]._rawSize;
		}

		byte* output = new byte[writeSize];

		uint64_t offset = 0;
		uint32_t last = _texInfos[texIdx]._blocks.size() - 1;

		offset = _texInfos[texIdx]._blocks[last]._blockOff;
		byte* gnf = new byte[0x100];
		fs.seekg(offset + 0x10, ios::beg);
		fs.read((char*)gnf, 0x100);

		memmove(output + writeOff, gnf, 0x100);
		writeOff += 0x100;

		for (int e = _texInfos[texIdx]._blocks.size() - 1; e >= 0; e--)
		{
			offset = _texInfos[texIdx]._blocks[e]._blockOff;

			uint64_t rawSize = _texInfos[texIdx]._blocks[e]._rawSize;
			byte* outbytes = new byte[rawSize + SAFE_SPACE];

			uint32_t off = 0;
			fs.seekg(offset + 4, ios::beg);
			fs.read((char*)&off, sizeof(uint32_t));

			uint32_t Size = _texInfos[texIdx]._blocks[e]._blockSize - off;

			byte* inbytes = new byte[Size];
			fs.seekg(offset + off, ios::beg);
			fs.read((char*)inbytes, Size);

			uint32_t status = OodLZ_Decompress(inbytes, Size, outbytes, rawSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			/*
			if (status == rawSize)
				cout << " Good";
			else
				cout << " bad";
			*/
			memmove(output + writeOff, outbytes, rawSize);
			writeOff += rawSize;
			delete[] inbytes;
			delete[] outbytes;
		}
		delete[] gnf;

		string f = dir + "\\" + std::to_string(_texInfos[i]._globHash) + ".gnf";
		ofstream fs = ofstream(f, ios::out | ios::binary);

		fs.write((char*)output, writeSize);
		fs.close();
		delete[] output;
	}
}