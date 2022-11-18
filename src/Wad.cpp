#include "pch.h"
#include "Wad.h"
#include <filesystem>
#include <iomanip>
#include <queue>
#include <map>
#include <cassert>

WADArchive::WADArchive()
{
    _streamSize = sizeof _header;
    _baseOffset = sizeof _header;
}

WADArchive::WADArchive(shared_ptr<iostream> instream)
{
    Read(instream);
}

bool WADArchive::Read(std::shared_ptr<std::iostream> instream)
{
    if (instream == nullptr || instream->fail())
        return false;

    stream = instream;

    stream->seekg(0, std::ios::end);
    _streamSize = stream->tellg();

    size_t te = sizeof _header;
    if (_streamSize < sizeof _header)
        return false;

    stream->seekg(0, std::ios::beg);
    stream->read((char*)&_header, sizeof _header);

    if (_header.magic != 0x434F5457)
        return false;
    if (_header.ver != 0x2)
        return false;

    if (_header.fileCount < 1)
        return true;

    if (_streamSize < sizeof _header + _header.fileCount * sizeof FileDesc)
    return false;
    
    _fileEntries = vector<FileDesc>(_header.fileCount);


    for (auto itr = _fileEntries.begin(); itr != _fileEntries.end(); itr++)
    {
        stream->read((char*)&(*itr), sizeof FileDesc);
    }
    _baseOffset = stream->tellg();

    _fileAbsOffsets = vector<size_t>(_header.fileCount);

    vector<uint32_t> sizesPadded(_header.fileCount);
    std::map<uint8_t, uint32_t> totalSize;
    
    totalSize[0] = _header.block0Size;
    totalSize[1] = _header.block1Size;
    totalSize[2] = _header.block2Size;
    totalSize[8] = _header.block8Size;

    for (long long i = _header.fileCount - 1; i >= 0; i--)
    {
        if (_fileEntries[i].nameStr() != "autopad")
        {
            sizesPadded[i] = totalSize[_fileEntries[i].blockBitSet] - _fileEntries[i].offset;
            totalSize[_fileEntries[i].blockBitSet] -= sizesPadded[i];
        }
    }

    std::queue<uint32_t> q;

    size_t readOff = _baseOffset;
    
    std::map<uint8_t,uint32_t> bitsetOffs;

    std::map<uint8_t, std::queue<uint32_t>> flushQ;
 
    for (uint32_t i = 0; i < _header.fileCount; i++)
    {
        if (_fileEntries[i].nameStr() == "autopad")
        {
            for (auto itr = flushQ.begin(); itr != flushQ.end(); itr++)
            {
                auto itr1 = itr;
                itr1++;
                readOff -= bitsetOffs[itr->first];
                if (!itr->second.empty())
                {
                        bitsetOffs[itr->first] = _fileEntries[itr->second.back()].offset + _fileEntries[itr->second.back()].size;
                    if(itr1 == flushQ.end())
                        bitsetOffs[itr->first] += _fileEntries[itr->second.back()].unk2[24];
                }
                while (!itr->second.empty())
                {
                    _fileAbsOffsets[itr->second.front()] = readOff + _fileEntries[itr->second.front()].offset;

                    itr->second.pop();
                }
                readOff += bitsetOffs[itr->first];
            }
            flushQ.clear();
            readOff += _fileEntries[i].size;
        }
        else
        {
            flushQ[_fileEntries[i].blockBitSet].push(i);
        }
    }
    {
        for (auto itr = flushQ.begin(); itr != flushQ.end(); itr++)
        {
            readOff -= bitsetOffs[itr->first];
            if (!itr->second.empty())
            {
                bitsetOffs[itr->first] = _fileEntries[itr->second.back()].offset + _fileEntries[itr->second.back()].size;
            }
            while (!itr->second.empty())
            {
                _fileAbsOffsets[itr->second.front()] = readOff + _fileEntries[itr->second.front()].offset;

                itr->second.pop();
            }
            readOff += bitsetOffs[itr->first];
        }
    }

    //for (uint32_t i = 0; i < _header.fileCount; i++)
    //{
    //    if (_fileEntries[i].nameStr() == "autopad")
    //    {
    //        for (auto itr = flushQ.begin(); itr != flushQ.end(); itr++)
    //        {
    //            while (!itr->second.empty())
    //            {
    //                _fileAbsOffsets[itr->second.front()] = readOff;

    //                readOff += sizesPadded[itr->second.front()];
    //                itr->second.pop();
    //            }
    //        }
    //        readOff += _fileEntries[i].size;
    //    }
    //    else
    //    {
    //        flushQ[_fileEntries[i].blockBitSet].push(i);
    //    }
    //}
    //{
    //    for (auto itr = flushQ.begin(); itr != flushQ.end(); itr++)
    //    {
    //        while (!itr->second.empty())
    //        {
    //            _fileAbsOffsets[itr->second.front()] = readOff;

    //            readOff += sizesPadded[itr->second.front()];
    //            itr->second.pop();
    //        }
    //    }
    //}

    for (uint32_t i = 0; i < _fileAbsOffsets.size(); i++)
    {
        if (_fileEntries[i].nameStr() == "DCClientGUID")
        {
            stream->seekg(_fileAbsOffsets[i]);
            for (int j = 0; j < _fileEntries[i].size - 2; j++)
            {
                char t;
                stream->read((char*)&t, 1);

                if (t > 44 && t < 91)
                {

                }
                else
                {
                    break;
                }
            }
        } 
    }
    return true;
}
bool WADArchive::GetFile(const uint32_t& entryIdx,shared_ptr <uint8_t[]> output) const
{
	
	if (entryIdx >= _fileEntries.size())
		return false;

    if (_fileEntries[entryIdx].size + _fileEntries[entryIdx].offset + _baseOffset > _streamSize)
        return false;
	output = make_shared<uint8_t[]>(_fileEntries[entryIdx].size);

	stream->seekg(_fileEntries[entryIdx].offset, std::ios::beg);
    stream->read((char*)output.get(), _fileEntries[entryIdx].size);
	
	return true;
	
}
bool WADArchive::GetFile(const uint32_t& entryIdx, std::iostream& outstream) const
{
	
    if (entryIdx >= _fileEntries.size())
        return false;

    if (_fileEntries[entryIdx].size + _fileEntries[entryIdx].offset + _baseOffset > _streamSize)
        return false;

	std::unique_ptr<uint8_t[]> output = std::make_unique<uint8_t[]>(_fileEntries[entryIdx].size);

    stream->seekg(_fileEntries[entryIdx].offset + _baseOffset, std::ios::beg);

    stream->read((char*)output.get(), _fileEntries[entryIdx].size);

	outstream.write((char*)output.get(), _fileEntries[entryIdx].size);
	
	return true;
}
bool WADArchive::UnpackFiles(const string& outDir)
{
    if (stream == nullptr || stream->fail())
        return false;

    for (uint32_t i = 0; i < _fileEntries.size(); i++)
    {
        path pth = path(outDir) / (_fileEntries[i].nameStr() + "_" + std::to_string(i) + ".bin");

        fstream ofs(pth, std::ios::binary | std::ios::out);

        GetFile(i, ofs);

        ofs.close();
    }
}
void WADArchive::WriteBufferToWad(const byte* buffer, const size_t& bufferSize, const size_t& bufferIdx)
{
    //size_t cnt = 0;
    //outWadStream.seekg(0, ios::end);
    //size_t end = outWadStream.tellg();

    //uint64_t pad = 0;
    //outWadStream.seekg(0, ios::beg);
    //while (outWadStream.tellg() < end)
    //{
    //    WadFile::FileDesc entry;
    //    outWadStream.read((char*)&entry.group, sizeof(uint16_t));
    //    outWadStream.read((char*)&entry.type, sizeof(uint16_t));
    //    size_t off = outWadStream.tellg();
    //    outWadStream.read((char*)&entry.size, sizeof(uint32_t));
    //    if (cnt == bufferIdx)
    //    {
    //        outWadStream.seekp(off);
    //        uint32_t tempSize = uint32_t(bufferSize);
    //        outWadStream.write((char*)&tempSize, sizeof(tempSize));
    //    }
    //    outWadStream.seekg(0x10, ios::cur);
    //    char name[0x38];
    //    outWadStream.read(name, sizeof(name));
    //    entry.name = string(name);
    //    outWadStream.seekg(0x10, ios::cur);
    //    if (entry.size != 0)
    //    {
    //        entry.offset = static_cast<uint32_t>(outWadStream.tellg());
    //        outWadStream.seekg(entry.size, ios::cur);
    //        pad = outWadStream.tellg();
    //        pad = (pad + 15) & (~15);
    //        outWadStream.seekg(pad, ios::beg);
    //        if (cnt == bufferIdx)
    //        {
    //            size_t forwardSize = end - pad;
    //            byte* forwardBytes = new byte[forwardSize];
    //            outWadStream.read((char*)forwardBytes, forwardSize);
    //            outWadStream.seekp(entry.offset);
    //            outWadStream.write((char*)buffer, bufferSize);
    //            size_t padSize = ((bufferSize + 15) & (~15)) - bufferSize;
    //            if (padSize > 0)
    //            {
    //                byte* paddingBytes = new byte[padSize];
    //                std::fill(paddingBytes, paddingBytes + padSize, 0);
    //                outWadStream.write((char*)paddingBytes, padSize);
    //                delete[] paddingBytes;
    //            }
    //            outWadStream.write((char*)forwardBytes, forwardSize);
    //            end = outWadStream.tellp();
    //            outWadStream.seekg(entry.offset + bufferSize + padSize, std::ios::beg);
    //            delete[] forwardBytes;
    //        }
    //        cnt++;
    //    }
    //}
}