#pragma once
#include <cstdint>
#include <memory>
#include <iostream>

using std::iostream;
using std::make_unique;
using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::filesystem::path;

class WADArchive
{
public:
	enum class FileType : uint16_t
	{
		GOWR_MESH_MAP = 0x0,
		GOWR_MESH_DEFN = 0x1,
		RigidMeshDefData = 0xB,
		GOWR_SMSH_DEFN,
		Rig = 0x3D,
		GOWR_GOPROTO_RIG = 0x3F,
		SkinnedMeshDef = 0x98,
		GOWR_MG_DEFN,
		GOWR_SHADER = 0x801E,
		Texture = 0x80A1,
		GOWR_TEXTURE,
		SkinnedMeshBuff = 0x8198,
		GOWR_MG_GPU_BUFF
	};
	struct Header
	{
		uint32_t magic = 0x434F5457;
		uint32_t ver = 0x2; // not sure
		uint32_t fileCount = 0;

		uint32_t unk1 = 0;
		uint32_t unk2 = 0;

		uint32_t block0Size = 0;
		uint32_t block1Size = 0;
		uint32_t block2Size = 0;

		uint8_t unk3[20] = {0};

		uint32_t block8Size = 0;

		uint32_t unk4 = 0;
		uint32_t unk5 = 0;
	};
	struct FileDesc
	{
		uint16_t group = 0;
		FileType type;
		uint32_t size = 0;

		uint8_t unk1[16] = { 0 };

	private:
		char name[0x38] = { 0 };

	public:
		uint8_t unk2[0x1F] = { 0 };

		uint8_t blockBitSet { 0 };

		uint8_t unk3[0x8] = { 0 };

		uint32_t offset = 0;

		uint8_t unk4[12] = { 0 };

		uint32_t offset2 = { 0 };

		uint8_t unk5[4] = { 0 };

		inline string nameStr() const
		{
			return string(name);
		}
	};
	Header _header;
	vector<FileDesc> _fileEntries;
	vector<size_t> _fileAbsOffsets;	

	bool Read(shared_ptr<std::iostream> instream);
	void WriteBufferToWad(const byte* buffer, const size_t& bufferSize, const size_t& bufferIdx);
	bool GetFile(const uint32_t& entryIdx, shared_ptr<uint8_t[]> output) const;
	bool GetFile(const uint32_t& entryIdx, std::iostream& outstream) const;
	bool UnpackFiles(const string& outDir);
	bool Test();
	void Dump(const string& outpath);
	WADArchive();
	WADArchive(shared_ptr<std::iostream> instream);

private:	
	std::shared_ptr<std::iostream> stream;
	size_t _streamSize;
	size_t _baseOffset;
};