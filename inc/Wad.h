#pragma once

struct WadFile
{
	vector<string> _Entries;
	vector<uint32_t> _Offsets;
	vector<uint32_t> _Sizes;
	vector<uint16_t> _Groups;
	bool Read(const std::filesystem::path& filepath);
	bool GetBuffer(const uint32_t& entryIdx, uint8_t* output);
	bool GetBuffer(const uint32_t& entryIdx, std::iostream& outstream);
private:
	ifstream fs;
};