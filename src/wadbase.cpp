#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <vector>

namespace fs = std::filesystem;
using std::ifstream;
using std::ostream;
using std::vector;

struct stcTop
{
	uint16_t int1;
	uint16_t int2;
	uint32_t int3;
	uint32_t int4;
};
struct Group
{
	uint64_t hash;
	uint64_t int1;
	uint64_t int2;
};
struct SubGroup
{
	uint32_t int1;
	uint32_t int2;
	uint64_t hash;
	uint64_t int3;
};

class A
{
	int arr[10];
};
//int main()
//{
//
//	ifstream ifs;
//	ifs.open(R"(C:\Users\abhin\OneDrive\Desktop\New folder (5)\r_baldur00\WAD_R_Baldur00.1.bin)", std::ios::binary | std::ios::in);
//
//	ifs.seekg(0x20, std::ios::beg);
//
//	uint32_t off1,cnt1;
//	ifs.read((char*)&off1, sizeof off1);
//	ifs.read((char*)&cnt1, sizeof cnt1);
//
//	vector<stcTop> stctop(cnt1);
//	for (auto itr = stctop.begin(); itr < stctop.end(); itr++)
//	{
//		ifs.read((char*)&(*itr), sizeof stcTop);
//	}
//
//	uint32_t groupCnt,subGroupCnt;
//	ifs.seekg(off1, std::ios::beg);
//
//	ifs.read((char*)&groupCnt, sizeof groupCnt);
//	ifs.read((char*)&subGroupCnt, sizeof subGroupCnt);
//
//	vector<Group> groups(groupCnt);
//	vector<SubGroup> subgroups(subGroupCnt);
//
//	ifs.seekg(0x10, std::ios::cur);
//	for (auto itr = groups.begin(); itr < groups.end() - 1;itr++)
//	{
//		ifs.read((char*)&(*itr), sizeof Group);
//	}
//	ifs.read((char*)&(groups.end() - 1)->hash, sizeof Group::hash);
//	ifs.read((char*)&(groups.end() - 1)->int1, sizeof Group::int1);
//
//	for (auto itr = subgroups.begin(); itr < subgroups.end(); itr++)
//	{
//		ifs.read((char*)&(*itr), sizeof SubGroup);
//	}
//}