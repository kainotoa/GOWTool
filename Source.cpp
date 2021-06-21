#include <pch.h>
#include "../inc/Wad.h"
#include <../inc/Texpack.h>
#include <chrono>  // for high_resolution_clock

int main(void)
{
	//LoadLib();
	/*
	ifstream fs;
	fs.open("", ios::binary | ios::in);

	uint32_t size = 670212;
	uint32_t rawsize = 1048576;


	byte* input = new byte[size];
	fs.seekg(288, ios::beg);
	fs.read((char*)input, size);

	byte* output = NULL;
	output = new byte[rawsize + 64];

	int outbytes = 0;
	outbytes = OodLZ_Decompress(input, size, output, rawsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	cout << outbytes;
	fs.close();
	ofstream os;

	os.open("", ios::binary | ios::out);
	os.write((char*)output, outbytes);
	os.close();
	

	string o = R"(D:\texs\a.gnf)";
	Texpack t = Texpack(f);
	ofstream ofs = ofstream(o, ios::out | ios::binary);

	uint32_t size = 0;
	byte* out = t.ExportTexture(11829719752652319193, size);

	ofs.write((char*)out, size);
	ofs.close();
	*/
	auto start = std::chrono::high_resolution_clock::now();

	string f = R"()";
	Texpack t = Texpack(f);
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	cout << elapsed.count();
	//t.ExportAll(R"()");




}