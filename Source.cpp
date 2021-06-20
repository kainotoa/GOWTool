#include <pch.h>
#include "../inc/Wad.h"
#include <../inc/Texpack.h>
#include <../inc/krak.h>

int main(void)
{
	LoadLib();

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
}