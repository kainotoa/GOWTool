#include "pch.h"
#include "Wad.h"
#include <filesystem>
#include <iomanip>
#include <queue>

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

    size_t size = 0;

    size_t block0Offset = _baseOffset;
    size_t block1Offset = 0;
    size_t block2Offset = 0;
    size_t block8Offset = 0;

    for (int i = _header.fileCount - 1; i >= 0; i--)
    {
        if (_fileEntries[i].blockBitSet == 0)
        {
            block1Offset = _fileEntries[i].offset + _fileEntries[i].size;
            break;
        }
    }

    block1Offset += block0Offset;

    for (int i = _header.fileCount - 1; i >= 0; i--)
    {
        if (_fileEntries[i].blockBitSet == 1)
        {
            block2Offset = _fileEntries[i].offset + _fileEntries[i].size;
            break;
        }
    }

    block2Offset += block1Offset;
    for (int i = _header.fileCount - 1; i >= 0; i--)
    {
        if (_fileEntries[i].blockBitSet == 2)
        {
            block8Offset = _fileEntries[i].offset + _fileEntries[i].size;
            break;
        }
    }
    block8Offset += block2Offset;

    size_t autopad_buffer1= 0;
    size_t autopad_buffer2 = 0;
    size_t autopad_buffer3 = 0;
    size_t autopad_buffer4 = 0;

    int cnt = 0;
    //std::fstream ss(R"(D:\gowr test\test1.txt)", std::ios::out);
    for (int i = 0; i < _header.fileCount; i++)
    {
        cout.width(56); cout << std::left << _fileEntries[i].nameStr();

        cout.width(4); cout << std::left << (int)_fileEntries[i].blockBitSet;
     
        cout.width(10); cout << std::left << (int)_fileEntries[i].type;

        cout << "\n";
        //if (_fileEntries[i].blockBitSet == 0)
        //{
        //    if ((i - 1) >= 0 && _fileEntries[i - 1].nameStr() == "autopad")
        //        autopad_buffer1 += _fileEntries[i - 1].size;
        //    if ((i + 1) < _header.fileCount && _fileEntries[i + 1].nameStr() == "autopad")
        //        autopad_buffer1 += _fileEntries[i + 1].size;
        //}
        //if (_fileEntries[i].blockBitSet == 8)
        //    cout << _fileEntries[i].nameStr() << "\n";
        //if (_fileEntries[i].blockBitSet == 0 && _fileEntries[i].offset == 854320)
        //    break;
        //if (_fileEntries[i].blockBitSet == 0)
        //    cnt++;
        ////if (_fileEntries[i].nameStr() == "autopad")
        ////{
        ////    switch (_fileEntries[i - 2].blockBitSet)
        ////    {
        ////    case 0:
        ////        autopad_buffer1 += _fileEntries[i].size;
        ////        break;
        ////    case 1:
        ////        autopad_buffer2 += _fileEntries[i].size;
        ////        break;
        ////    case 2:
        ////        autopad_buffer3 += _fileEntries[i].size;

        ////        break;
        ////    case 8:
        ////        autopad_buffer4 += _fileEntries[i].size;

        ////        break;
        ////    default:
        ////        break;
        ////    }
        ////}
        //if (_fileEntries[i].nameStr() == "autopad")
        //{
        //    cnt++;
        //}
        //ss.width(56); ss << std::left << _fileEntries[i].nameStr();
        //ss.width(10); ss << std::left <<_fileEntries[i].offset;
        //ss.width(10); ss << std::left <<_fileEntries[i].size;

        //for (int j = 0; j < 0x1F; j++)
        //{
        //    ss.width(4); ss << std::left <<uint16_t(_fileEntries[i].unk2[j]);
        //}
        //ss.width(4); ss << std::left << (int)_fileEntries[i].blockBitSet;
        //for (int j = 0; j < 0x8; j++)
        //{
        //    ss.width(4); ss << std::left <<(int)_fileEntries[i].unk3[j];
        //}
        //for (int j = 0; j < 0x14; j++)
        //{
        //    ss.width(4); ss << std::left << (int)_fileEntries[i].unk4[j];
        //}
        //ss << "\n";
    }

    _fileAbsOffsets = vector<size_t>(_header.fileCount);

    std::queue<uint32_t> q;

    size_t readOff = _baseOffset;
    
    size_t corrLastOffsetBitset0 = 0;
    size_t corrLastOffsetBitset1 = 0;
    size_t corrLastOffsetBitset2 = 0;
    size_t corrLastOffsetBitset8 = 0;
    
    uint8_t directBitSet = _fileEntries[0].blockBitSet;
    uint8_t queueBitSet = 1;
    
    for (uint32_t i = 1; i < _header.fileCount; i++)
    {
        if (directBitSet != _fileEntries[i].blockBitSet)
        {
            queueBitSet = _fileEntries[i].blockBitSet;
            break;
        }
    }
    for (uint32_t i = 0; i < _header.fileCount; i++)
    {
        if (_fileEntries[i].nameStr() == "autopad")
        {
            uint32_t temp = 0;
            switch (directBitSet)
            {
            case 0:
                temp = corrLastOffsetBitset0;
                break;
            case 1:
                temp = corrLastOffsetBitset1;
                break;
            case 2:
                temp = corrLastOffsetBitset2;
                break;
            case 8:
                temp = corrLastOffsetBitset8;
                break;
            default:
                break;
            }
            readOff += temp;

            uint32_t temp2 = 0;

            switch (queueBitSet)
            {
            case 0:
                temp2 = corrLastOffsetBitset0;
                break;
            case 1:
                temp2 = corrLastOffsetBitset1;
                break;
            case 2:
                temp2 = corrLastOffsetBitset2;
                break;
            case 8:
                temp2 = corrLastOffsetBitset8;
                break;
            default:
                break;
            }

            if (!q.empty())
            {
                readOff  -= temp2;
                temp2 = _fileEntries[q.back()].offset + _fileEntries[q.back()].size;

                while (!q.empty())
                {
                    _fileAbsOffsets[q.front()] = readOff + _fileEntries[q.front()].offset;
                    q.pop();
                }

                readOff += temp2;
            }

            switch (queueBitSet)
            {
            case 0:
                corrLastOffsetBitset0 = temp2;
                break;
            case 1:
                corrLastOffsetBitset1 = temp2;
                break;
            case 2:
                corrLastOffsetBitset2 = temp2;
                break;
            case 8:
                corrLastOffsetBitset8 = temp2;
                break;
            default:
                break;
            }

            readOff -= temp;
            readOff += _fileEntries[i].size;
        }
        else if(directBitSet == _fileEntries[i].blockBitSet)
        {
            _fileAbsOffsets[i] = readOff + _fileEntries[i].offset;

            switch (directBitSet)
            {
            case 0:
                corrLastOffsetBitset0 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            case 1:
                corrLastOffsetBitset1 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            case 2:
                corrLastOffsetBitset2 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            case 8:
                corrLastOffsetBitset8 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            default:
                break;
            }
        }
        else if(queueBitSet == _fileEntries[i].blockBitSet)
        {
            q.push(i);
        }
        else
        {
            uint32_t temp = 0;
            switch (directBitSet)
            {
            case 0:
                temp = corrLastOffsetBitset0;
                break;
            case 1:
                temp = corrLastOffsetBitset1;
                break;
            case 2:
                temp = corrLastOffsetBitset2;
                break;
            case 8:
                temp = corrLastOffsetBitset8;
                break;
            default:
                break;
            }
            readOff += temp;
            
            uint32_t temp2 = 0;

            switch (queueBitSet)
            {
            case 0:
                temp2 = corrLastOffsetBitset0;
                break;
            case 1:
                temp2 = corrLastOffsetBitset1;
                break;
            case 2:
                temp2 = corrLastOffsetBitset2;
                break;
            case 8:
                temp2 = corrLastOffsetBitset8;
                break;
            default:
                break;
            }

            if (!q.empty())
            {
                readOff -= temp2;
                temp2 = _fileEntries[q.back()].offset + _fileEntries[q.back()].size;

                while (!q.empty())
                {
                    _fileAbsOffsets[q.front()] = readOff + _fileEntries[q.front()].offset;
                    q.pop();
                }

                readOff += temp2;
            }

            switch (queueBitSet)
            {
            case 0:
                corrLastOffsetBitset0 = temp2;
                break;
            case 1:
                corrLastOffsetBitset1 = temp2;
                break;
            case 2:
                corrLastOffsetBitset2 = temp2;
                break;
            case 8:
                corrLastOffsetBitset8 = temp2;
                break;
            default:
                break;
            }


            directBitSet = _fileEntries[i].blockBitSet;

            for(uint32_t j = i + 1; j < _header.fileCount; j++)
                if (directBitSet != _fileEntries[j].blockBitSet)
                {
                    queueBitSet = _fileEntries[j].blockBitSet;
                    break;
                }

            switch (directBitSet)
            {
            case 0:
                readOff -= corrLastOffsetBitset0;
                break;
            case 1:
                readOff -= corrLastOffsetBitset1;
                break;
            case 2:
                readOff -= corrLastOffsetBitset2;
                break;
            case 8:
                readOff -= corrLastOffsetBitset8;
                break;
            default:
                break;
            }

            _fileAbsOffsets[i] = readOff + _fileEntries[i].offset;

            switch (directBitSet)
            {
            case 0:
                corrLastOffsetBitset0 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            case 1:
                corrLastOffsetBitset1 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            case 2:
                corrLastOffsetBitset2 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            case 8:
                corrLastOffsetBitset8 = _fileEntries[i].offset + _fileEntries[i].size;
                break;
            default:
                break;
            }
        }
    }
    //ss.close();
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