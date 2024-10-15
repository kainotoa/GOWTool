#include "pch.h"
#include "Wad.h"
#include <filesystem>
#include <queue>
#include <map>
#include <cassert>
#include <iomanip>

WADArchive::WADArchive()
{
    _streamSize = sizeof _header;
    _baseOffset = sizeof _header;
}

WADArchive::WADArchive(shared_ptr<iostream> instream)
{
    Read(instream);
}

void WADArchive::Dump(const string& outpath)
{
    fstream fs(outpath, ios::out);
    for (uint32_t i = 0; i < _header.fileCount; i++)
    {
        fs.width(56); fs << std::left << _fileEntries[i].nameStr();
        fs.width(9); fs << std::left << _fileEntries[i].offset;
        fs.width(9); fs << std::left << _fileEntries[i].size;
        fs.width(9); fs << std::left << _fileEntries[i].offset2;

        for (int j = 0; j < 0x1F; j++)
        {
            fs.width(4); fs << std::left << (int)_fileEntries[i].unk2[j];
        }
        fs.width(4); fs << std::left << (int)_fileEntries[i].blockBitSet;

        for (int j = 0; j < 0x8; j++)
        {
            fs.width(4); fs << std::left << (int)_fileEntries[i].unk3[j];
        }
        for (int j = 0; j < 12; j++)
        {
            fs.width(4); fs << std::left << (int)_fileEntries[i].unk4[j];
        }
        for (int j = 0; j < 4; j++)
        {
            fs.width(4); fs << std::left << (int)_fileEntries[i].unk5[j];
        }
        fs << "\n";
    }
    fs.close();
}
bool WADArchive::Test()
{
    for (uint32_t i = 0; i < _fileEntries.size(); i++)
    {
        if (_fileEntries[i].size + _fileAbsOffsets[i] > _streamSize)
            return false;
    }
    for (uint32_t i = 0; i < _fileAbsOffsets.size(); i++)
    {
        if (_fileEntries[i].nameStr() == "DCClientGUID")
        {
            stream->seekg(_fileAbsOffsets[i]);
            for (int j = 0; j < _fileEntries[i].size - 1; j++)
            {
                char t;
                stream->read((char*)&t, 1);

                if (t != '.' && t != '-' && !(t >= '0' && t <= '9') && !(t >= 'A' && t <= 'Z') && !(t >= 'a' && t <= 'z'))
                {
                    return false;
                }
            }
        }
        if (_fileEntries[i].type == WADArchive::FileType::GOWR_MESH_DEFN && _fileEntries[i].nameStr().rfind("MESH_", 0) == 0)
        {
            stream->seekg(_fileAbsOffsets[i]);
            uint32_t temp = 0;
            stream->read((char*)&temp, sizeof uint32_t);
            if (temp != 655372)
            {
                return false;
            }
        }
        if (_fileEntries[i].type == WADArchive::FileType::GOWR_MESH_DEFN && _fileEntries[i].nameStr().rfind("MG_", 0) == 0)
        {
            stream->seekg(_fileAbsOffsets[i]);
            uint32_t temp = 0;
            stream->read((char*)&temp, sizeof uint32_t);
            if (temp != 65548)
            {
                return false;
            }
        }
        // fails on file 59 in r_system.wad rest should pass
        if (_fileEntries[i].type == WADArchive::FileType::GOWR_SHADER)
        {
            stream->seekg(_fileAbsOffsets[i] + _fileEntries[i].size - 28);
            char arr[8] { 0 };
            stream->read((char*)&arr, 7);
            if (string(arr) != "OrbShdr")
            {
                return false;
            }
        }
    }

    return true;
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

    //vector<uint32_t> sizesPadded(_header.fileCount);
    //std::map<uint8_t, uint32_t> totalSize;
    //
    //totalSize[0] = _header.block0Size;
    //totalSize[1] = _header.block1Size;
    //totalSize[2] = _header.block2Size;
    //totalSize[8] = _header.block8Size;

    //for (long long i = _header.fileCount - 1; i >= 0; i--)
    //{
    //    if (_fileEntries[i].nameStr() != "autopad")
    //    {
    //        sizesPadded[i] = totalSize[_fileEntries[i].blockBitSet] - _fileEntries[i].offset;
    //        totalSize[_fileEntries[i].blockBitSet] -= sizesPadded[i];
    //    }
    //}

    //for (uint32_t i = 0; i < _header.fileCount; i++)
    //{
    //    if (_fileEntries[i].unk2[20] == 0 || _fileEntries[i].unk2[20] == 16)
    //    {

    //    }
    //    else
    //    {
    //        cout << "";
    //    }
    //}

    std::queue<uint32_t> q;

    size_t readOff = _baseOffset;
    
    std::map<uint8_t,uint32_t> bitsetOffs;

    std::map<uint8_t, std::queue<uint32_t>> flushQ;
 
    for (uint32_t i = 0; i < _header.fileCount; i++)
    {
        if (_fileEntries[i].unk3[0x2] == 1)
        {
            if(_fileEntries[i].nameStr() != "autopad")
                flushQ[_fileEntries[i].blockBitSet].push(i);
            if (_fileEntries[i].unk2[20] != 0)
                flushQ[8].push(i);

            for (auto itr = flushQ.begin(); itr != flushQ.end(); itr++)
            {
                readOff -= bitsetOffs[itr->first];
                uint32_t temp = bitsetOffs[itr->first];
                if (!itr->second.empty())
                {
                    while (!itr->second.empty())
                    {
                        if (itr->first == 8 && _fileEntries[itr->second.front()].blockBitSet != 8)
                        {
                            //_fileAbsOffsets[itr->second.front()] = readOff + _fileEntries[itr->second.front()].offset2;
                            temp = _fileEntries[itr->second.front()].offset2 + 16;
                            bitsetOffs[itr->first] = _fileEntries[itr->second.front()].offset2 + 16;
                        }
                        else
                        {
                            _fileAbsOffsets[itr->second.front()] = readOff + _fileEntries[itr->second.front()].offset;
                            bitsetOffs[itr->first] = _fileEntries[itr->second.front()].offset + _fileEntries[itr->second.front()].size;
                            temp = _fileEntries[itr->second.front()].offset + _fileEntries[itr->second.front()].size;
                        }
                        itr->second.pop();
                    }
                    
                }
                readOff += temp;
            }
            if (_fileEntries[i].nameStr() == "autopad")
            {
                _fileAbsOffsets[i] = readOff;
                readOff += _fileEntries[i].size;
            }
        }
        else
        {
            flushQ[_fileEntries[i].blockBitSet].push(i);
            if(_fileEntries[i].unk2[20] != 0)
                flushQ[8].push(i);
        }
    }
    {
        for (auto itr = flushQ.begin(); itr != flushQ.end(); itr++)
        {
            if (!itr->second.empty())
            {
                readOff -= bitsetOffs[itr->first];
                uint32_t temp = 0;

                while (!itr->second.empty())
                {
                    if (itr->first == 8 && _fileEntries[itr->second.front()].blockBitSet != 8)
                    {
                        //_fileAbsOffsets[itr->second.front()] = readOff + _fileEntries[itr->second.front()].offset2;
                        temp = _fileEntries[itr->second.front()].offset2 + 16;
                        bitsetOffs[itr->first] = _fileEntries[itr->second.front()].offset2 + 16;
                    }
                    else
                    {
                        _fileAbsOffsets[itr->second.front()] = readOff + _fileEntries[itr->second.front()].offset;
                        bitsetOffs[itr->first] = _fileEntries[itr->second.front()].offset + _fileEntries[itr->second.front()].size;
                        temp = _fileEntries[itr->second.front()].offset + _fileEntries[itr->second.front()].size;
                    }
                    itr->second.pop();
                }

                readOff += temp;
            }
        }
    }
    return true;
}
bool WADArchive::GetFile(const uint32_t& entryIdx,shared_ptr <uint8_t[]> output) const
{
	
	if (entryIdx >= _fileEntries.size())
		return false;

    if (_fileEntries[entryIdx].size + _fileAbsOffsets[entryIdx] > _streamSize)
        return false;
	output = make_shared<uint8_t[]>(_fileEntries[entryIdx].size);

	stream->seekg(_fileAbsOffsets[entryIdx], std::ios::beg);
    stream->read((char*)output.get(), _fileEntries[entryIdx].size);
	
	return true;
	
}
bool WADArchive::GetFile(const uint32_t& entryIdx, std::iostream& outstream) const
{
	
    if (entryIdx >= _fileEntries.size())
        return false;

    if (_fileEntries[entryIdx].size + _fileAbsOffsets[entryIdx] > _streamSize)
        return false;

	std::unique_ptr<uint8_t[]> output = std::make_unique<uint8_t[]>(_fileEntries[entryIdx].size);

    stream->seekg(_fileAbsOffsets[entryIdx], std::ios::beg);

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
        path pth = path(outDir) / (_fileEntries[i].nameStr() + "---" + std::to_string(i) + ".bin");

        fstream ofs(pth, std::ios::binary | std::ios::out);

        if(!GetFile(i, ofs))
            return false;

        ofs.close();
    }
    return true;
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