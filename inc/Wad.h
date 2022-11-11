#pragma once
#include <iostream>
#include <memory>

class WADArchive
{
public:
	struct Header
	{
		uint32_t magic = 0x434F5457;
		uint32_t ver = 0x2; // not sure
		uint32_t fileCount;

		uint32_t unk1;
		uint32_t unk2;

		uint32_t block1Size;
		uint32_t block2Size;
		uint32_t block3Size;

		uint8_t unk3[20];

		uint32_t block4Size;

		uint32_t unk4;
		uint32_t unk5;
	};
	struct FileDesc
	{
		enum class FileType : uint16_t
		{
			None = 0x0,
			RigidMeshDefData = 0xB,
			Rig = 0x3D,
			SkinnedMeshDef = 0x98,
			Texture = 0x80A1,
			SkinnedMeshBuff = 0x8198,
		};
		uint16_t group;
		FileType type;
		uint32_t size;

		uint8_t unk1[16];

		char name[0x38];

		uint8_t unk2[0x28];

		uint32_t offset;

		uint8_t unk3[0x14];
	};
	Header _header;
	vector<FileDesc> _FileEntries;
	std::shared_ptr<std::iostream> stream;
	bool Read(std::shared_ptr<std::iostream> instream);
	static void WriteBufferToWad(std::iostream& outWadStream, const byte* buffer, const size_t& bufferSize, const size_t& bufferIdx);
	bool GetFile(const uint32_t& entryIdx, uint8_t* output);
	bool GetFile(const uint32_t& entryIdx, std::iostream& outstream);
	WADArchive();
	WADArchive(const std::iostream& stream);


};