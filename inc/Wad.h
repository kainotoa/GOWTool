#pragma once

struct WADFile
{
	vector<string> _Entries;
	vector<uint32_t> _Offsets;
	vector<uint32_t> _Sizes;
	vector<uint16_t> _Groups;
	
	WADFile(string);
};