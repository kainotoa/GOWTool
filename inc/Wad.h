#pragma once

struct WadFile
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
	struct FileDesc
	{
		std::string name;
		uint16_t group;
		FileType type;
		uint32_t size;
		uint32_t offset;
	};
	vector<FileDesc> _FileEntries;
	bool Read(const std::filesystem::path& filepath);
	static void WriteBufferToWad(std::iostream& outWadStream, const byte* buffer, const size_t& bufferSize, const size_t& bufferIdx);
	bool GetBuffer(const uint32_t& entryIdx, uint8_t* output);
	bool GetBuffer(const uint32_t& entryIdx, std::iostream& outstream);
private:
	ifstream fs;
};