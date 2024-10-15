#include "Utils.h"
#include "pch.h"
#include "Rig.h"
#include "glTFSerializer.h"
#include "Formats.h"
#include "MainFunctions.h"
#include "Texpack.h"
#include "Lodpak.h"
#include "Wad.h"
#include "Gnf.h"
#include "converter.h"
#include "glTFDeserializer.h"
#include <map>
#include "krak.h"
#include "lz4.h"


using std::filesystem::path;
using std::filesystem::directory_iterator;
using std::filesystem::directory_entry;
using std::filesystem::recursive_directory_iterator;

using Utils::CommandLine;


extern std::map<PrimitiveTypes, std::set<std::pair<DataTypes, int>>> compMap;
extern std::set<DataTypes> dataSet;
//bool ImportModels(const std::filesystem::path& wadDir, const std::filesystem::path& wadPath, vector<Lodpack*>& lodpacks)
//{
//    std::map<uint64_t, std::stringstream*> buffersHashmap;
//
//    std::ifstream inWadStream(wadPath.string(), std::ios::binary | std::ios::in);
//    inWadStream.seekg(0, std::ios::end);
//    size_t inWadSize = inWadStream.tellg();
//
//    std::filesystem::path outWadPath = wadDir.parent_path() / (wadDir.stem().string() + ".wad");
//    std::fstream wadStream;
//    wadStream.open(outWadPath, std::ios::binary | std::ios::out);
//    wadStream.close();
//    wadStream.open(outWadPath, std::ios::binary | std::ios::in | std::ios::out);
//    byte* wadBytes = new byte[inWadSize];
//    inWadStream.seekg(0, std::ios::beg);
//    inWadStream.read((char*) wadBytes, inWadSize);
//    wadStream.write((char*)wadBytes, inWadSize);
//    delete[] wadBytes;
//    inWadStream.close();
//
//    vector<int> usedMeshBufferIndices;
//    WadFile srcWad;
//    srcWad.Read(wadPath);
//    std::filesystem::directory_iterator dirIter(wadDir);
//    for (const std::filesystem::directory_entry& iter : dirIter)
//    {
//        string ext = Utils::str_tolower(iter.path().filename().extension().string());
//        if (ext != ".glb" && ext != ".gltf")
//            continue;
//
//        string name = iter.path().filename().stem().string();
//
//        size_t defIdx = name.find_last_of('.');
//        if (defIdx == string::npos)
//        {
//            cout << "file index not apped to the file: " << name << "\n";
//            continue;
//        }
//
//        try
//        {
//            defIdx = std::stoull(name.substr(defIdx + 1, name.length() - defIdx - 1));
//        }
//        catch (...)
//        {
//            cout << "Invalid File Index Appended: " << name << "\n";
//            continue;
//        }
//        if (defIdx >= srcWad._FileEntries.size())
//        {
//            cout << "Invalid File Index Appended(Out of Bounds of Wad files): " << name << "\n";
//            continue;
//        }
//        name = name.substr(0, name.find_last_of('.'));
//        if (Utils::str_tolower(srcWad._FileEntries[defIdx].name) != Utils::str_tolower(name))
//        {
//            cout << "File name Doesn't match the original: " << name << "\n";
//            continue;
//        }
//        if (srcWad._FileEntries[defIdx].type != WadFile::FileType::SkinnedMeshDef)
//        {
//            cout << "Importing Skinned mesh is only possible atm: " << name << "\n";
//            continue;
//        }
//        std::shared_ptr<Microsoft::glTF::GLTFResourceReader> resourceReader;
//        Microsoft::glTF::Document document;
//        ReadGLTF(iter.path(), document, resourceReader);
//        //PrintDocumentInfo(document);
//
//        name = srcWad._FileEntries[defIdx].name.substr(3, srcWad._FileEntries[defIdx].name.length() - 5);
//        std::stringstream meshDefStream;
//        std::stringstream meshBuffStream;
//        std::stringstream rigStream;
//        srcWad.GetBuffer(defIdx, meshDefStream);
//        
//        int idx2 = 0;
//        for (int j = 0; j < srcWad._FileEntries.size(); j++)
//        {
//            if (srcWad._FileEntries[j].type == WadFile::FileType::SkinnedMeshBuff && (srcWad._FileEntries[j].name.find(srcWad._FileEntries[defIdx].name) != std::string::npos))
//            {
//                if (std::find(usedMeshBufferIndices.begin(), usedMeshBufferIndices.end(), j) != usedMeshBufferIndices.end())
//                    continue;
//                idx2 = j;
//                srcWad.GetBuffer(j, meshBuffStream);
//                usedMeshBufferIndices.push_back(j);
//                break;
//            }
//        }
//        
//        for (int j = 0; j < srcWad._FileEntries.size(); j++)
//        {
//            if (srcWad._FileEntries[j].type == WadFile::FileType::Rig && (srcWad._FileEntries[j].name.find("Proto") != std::string::npos))
//            {
//                std::string sample = Utils::str_tolower(srcWad._FileEntries[j].name.substr(7, srcWad._FileEntries[j].name.length() - 7));
//                if (sample == name)
//                {
//                    srcWad.GetBuffer(j, rigStream);
//                    break;
//                }
//            }
//        }
//
//        
//        MGDefinition meshDef;
//        auto oldMeshInfos = meshDef.ReadMG(meshDefStream);
//
//        Rig rig(rigStream);
//        std::vector<RawMeshContainer> inMeshes;
//        GetRawMeshesFromGLTF(document, *resourceReader,inMeshes, rig);
//
//        std::vector<MeshInfo> newMeshInfos;
//        for (uint32_t i = 0; i < inMeshes.size(); i++)
//        {
//            for (uint32_t j = 0; j < oldMeshInfos.size(); j++)
//            {
//                char buf[10];
//                sprintf_s(buf, "%04d", j);
//                string subname = "submesh_" + string(buf) + "_" + std::to_string(oldMeshInfos[j].LODlvl);
//                if (subname == inMeshes[i].name)
//                {
//                    std::stringstream* buffer;
//                    MeshInfo newMeshInfo = oldMeshInfos[j];
//
//                    if (inMeshes[i].VertCount > newMeshInfo.vertCount || inMeshes[i].IndCount > newMeshInfo.indCount)
//                    {
//                        break;
//                        std::cout << "";
//                    }
//
//                    newMeshInfo.name = subname;
//                    if (newMeshInfo.Hash != 0)
//                    {
//                        if (buffersHashmap.contains(newMeshInfo.Hash))
//                            buffer = buffersHashmap.at(newMeshInfo.Hash);
//                        else
//                        {
//                            buffer = new std::stringstream(std::ios::binary | std::ios::out | std::ios::in);
//                            for (int k = 0; k < lodpacks.size(); k++)
//                            {
//                                if (lodpacks[k]->GetBuffer(newMeshInfo.Hash, *buffer))
//                                    break;
//                            }
//                            buffersHashmap.insert({ newMeshInfo.Hash, buffer });
//                        }
//                        size_t writeOff = newMeshInfo.vertexOffset;
//                        WriteRawMeshToStream(newMeshInfo, inMeshes[i], *buffer, writeOff);
//                        newMeshInfo.vertCount = oldMeshInfos[j].vertCount;
//                        newMeshInfos.push_back(newMeshInfo);
//                    }
//                    else
//                    {
//                        size_t writeOff = newMeshInfo.vertexOffset;
//                        WriteRawMeshToStream(newMeshInfo, inMeshes[i], meshBuffStream, writeOff);
//                        newMeshInfo.vertCount = oldMeshInfos[j].vertCount;
//                        newMeshInfos.push_back(newMeshInfo);
//                    }
//                    break;
//                }
//            }
//        }
//        meshDef.WriteMG(newMeshInfos, meshDefStream);
//        size_t defSize = srcWad._FileEntries[defIdx].size;
//        byte* defbytes = new byte[defSize];
//        meshDefStream.seekg(0, std::ios::beg);
//        meshDefStream.read((char*)defbytes, defSize);
//
//
//        WadFile::WriteBufferToWad(wadStream, defbytes, defSize, defIdx);
//
//        size_t buffSize = srcWad._FileEntries[idx2].size;
//        byte* buffBytes = new byte[buffSize];
//        meshBuffStream.seekg(0, std::ios::beg);
//        meshBuffStream.read((char*)buffBytes, buffSize);
//
//        WadFile::WriteBufferToWad(wadStream, buffBytes, buffSize, idx2);
//
//        delete[] defbytes;
//        delete[] buffBytes;
//    }
//    wadStream.close();
//
//    //std::filesystem::path outlodpackPath = wadDir.parent_path() / (wadDir.stem().string() + ".lodpack");
//
//    //Lodpack outlodpack;
//    //outlodpack.Write(outlodpackPath, buffersHashmap);
//
//    for (auto itr = buffersHashmap.begin(); itr != buffersHashmap.end(); itr++)
//    {
//        for (int k = 0; k < lodpacks.size(); k++)
//        {
//            if (lodpacks[k]->SetBuffer(itr->first, *itr->second))
//                break;
//        }
//    }
//    for (int k = 0; k < lodpacks.size(); k++)
//    {
//        lodpacks[k]->file.close();
//    }
//    return true;
//}
bool ImportAllGnf(const std::filesystem::path& gnfSrcDir, vector<Texpack*>& texpacks, const std::filesystem::path& gamedir)
{
    if (gnfSrcDir.empty() || !gnfSrcDir.is_absolute() || !std::filesystem::exists(gnfSrcDir) || !std::filesystem::is_directory(gnfSrcDir))
    {
        return false;
    }

    std::vector<Gnf::GnfImage*> gnfImages;
    std::vector<uint64_t> gnfHashes;
    std::filesystem::directory_iterator dir(gnfSrcDir);
    for (const std::filesystem::directory_entry& entry : dir)
    {
        Gnf::GnfImage* gnfImage = new Gnf::GnfImage();
        if (Utils::str_tolower(entry.path().extension().string()) == ".gnf")
        {
            std::ifstream ifs(entry.path().string(), std::ios::binary | std::ios::in);
            ifs.seekg(0x0, std::ios::end);
            size_t size = ifs.tellg();
            ifs.seekg(0x0, std::ios::beg);

            if (size < 0x200)
            {
                continue;
                ifs.close();
            }

            uint32_t magic;
            ifs.read((char*)&magic, sizeof(uint32_t));
            if (magic != gnfImage->header.gnfMagic)
            {
                continue;
                ifs.close();
            }

            ifs.seekg(0x0, std::ios::beg);

            byte* bytes = new byte[size];
            ifs.read((char*)bytes, size);
            ifs.close();

            try
            {
                gnfImage->ReadImage(bytes);
            }
            catch (std::exception& ex)
            {
                continue;
            }

            gnfImages.push_back(gnfImage);
            delete[] bytes;
        }
        else if (Utils::str_tolower(entry.path().extension().string()) == ".dds")
        {
            std::ifstream ifs(entry.path().string(), std::ios::binary | std::ios::in);
            ifs.seekg(0x0, std::ios::end);
            size_t size = ifs.tellg();
            ifs.seekg(0x0, std::ios::beg);

            if (size < 0x95)
            {
                continue;
                ifs.close();
            }

            uint32_t magic;
            ifs.read((char*)&magic, sizeof(uint32_t));
            
            if (magic != 0x20534444)
            {
                continue;
                ifs.close();
            }
            
            ifs.seekg(0x0, std::ios::beg);

            byte* ddsbytes = new byte[size];
            ifs.read((char*)ddsbytes, size);
            ifs.close();

            try
            {
                byte* bytes = nullptr;
                size = ConvertDDSToGnf(ddsbytes, size, bytes);
                gnfImage->ReadImage(bytes);

                delete[] bytes;
            }
            catch (const std::exception& ex)
            {
                continue;
            }

            gnfImages.push_back(gnfImage);
            delete[] ddsbytes;
        }
        else
            continue;

        try
        {
            std::stringstream s;
            std::string str = entry.path().filename().stem().string();
            if (str.find("TX_") != std::string::npos)
            {
                str = str.substr(str.find_last_of("_") + 1, 16);
                if (str.length() < 16)
                    continue;
                s << std::hex << str;
            }
            else
            {
                s << str;
            }
            uint64_t hash = 0;
            s >> hash;

            gnfHashes.push_back(hash);
        }
        catch (const std::exception& ex)
        {
            continue;
        }
    }

    if (gnfImages.size() < 1)
        return false;

    if (gnfImages.size() != gnfHashes.size())
        return false;

    {
        //std::filesystem::recursive_directory_iterator dir1(gamedir);
        //for (const std::filesystem::directory_entry& entry : dir1)
        //{
        //    if (entry.path().extension().string() == ".wad")
        //    {
        //        WadFile wad;
        //        wad.Read(entry.path());
        //        for (int i = 0; i < wad._FileEntries.size(); i++)
        //        {
        //            if ((int)wad._FileEntries[i].type == 32801)
        //            {
        //                std::stringstream s;
        //                std::string name = wad._FileEntries[i].name.substr(3);
        //                s << std::hex << name.substr(name.find_last_of("_") + 1, 16);
        //                uint64_t hash = 0;
        //                s >> hash;
        //                for (int j = 0; j < gnfHashes.size(); j++)
        //                {
        //                    if (hash == gnfHashes[j])
        //                    {
        //                        size_t offptr = wad._FileEntries[i].offset;
        //                        offptr += 0x47;
        //                        byte byteptr = 0xE;
        //                        wad.fs.seekp(offptr, std::ios::beg);

        //                        wad.fs.write((char*)&byteptr, sizeof byteptr);

        //                        uint16_t maxRes = 8192;

        //                        wad.fs.write((char*)&maxRes, sizeof maxRes);
        //                        wad.fs.write((char*)&maxRes, sizeof maxRes);

        //                        wad.fs.seekp(0xA, std::ios::cur);

        //                        switch (gnfImages[j]->header.format)
        //                        {
        //                        case Gnf::Format::FormatBC1:
        //                        case Gnf::Format::Format8:
        //                        case Gnf::Format::FormatBC4:
        //                            byteptr = 0x3;
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            byteptr = 0xB;
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            byteptr = 0x2B;
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            break;
        //                        case Gnf::Format::FormatBC2:
        //                        case Gnf::Format::FormatBC3:
        //                        case Gnf::Format::FormatBC7:
        //                        case Gnf::Format::FormatBC5:
        //                        case Gnf::Format::FormatBC6:
        //                            byteptr = 0x6;
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            byteptr = 0x16;
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            byteptr = 0x56;
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            wad.fs.write((char*)&byteptr, sizeof byteptr);
        //                            break;
        //                        default:
        //                            throw std::exception("Format not implemented!");
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //        wad.fs.close();
        //    }
        //}
    }
    std::filesystem::path outTexpackPath = gnfSrcDir.parent_path() / (gnfSrcDir.filename().string() + ".texpack");
    std::filesystem::path outTexpackTocPath = gnfSrcDir.parent_path() / (gnfSrcDir.filename().string() + ".texpack.toc");

    std::ofstream ofs(outTexpackPath.string(), std::ios::binary | std::ios::out);
    std::ofstream ofs1(outTexpackTocPath.string(), std::ios::binary | std::ios::out);

    for (size_t i = 0; i < 4; i++)
    {
        uint64_t z = 0x0ULL;
        ofs.write((char*)&z, sizeof(z));
        ofs1.write((char*)&z, sizeof(z));
    }

    uint32_t _texSectionOff = (0x38 + gnfImages.size() * 0x18 + gnfImages.size() * 0x20 + 15) & (~15);
    uint32_t _blocksCount = gnfImages.size();
    uint32_t _blocksInfoOff = 0x38 + gnfImages.size() * 0x18;
    uint32_t _TexsCount = gnfImages.size();

    ofs.write((char*)&_texSectionOff, sizeof(_texSectionOff));
    ofs.write((char*)&_blocksCount, sizeof(_blocksCount));
    ofs.write((char*)&_blocksInfoOff, sizeof(_blocksInfoOff));
    ofs.write((char*)&_TexsCount, sizeof(_TexsCount));
    uint64_t z = 0x5ULL;
    ofs.write((char*)&z, sizeof(z));

    ofs1.write((char*)&_texSectionOff, sizeof(_texSectionOff));
    ofs1.write((char*)&_blocksCount, sizeof(_blocksCount));
    ofs1.write((char*)&_blocksInfoOff, sizeof(_blocksInfoOff));
    ofs1.write((char*)&_TexsCount, sizeof(_TexsCount));
    ofs1.write((char*)&z, sizeof(z));

    Texpack::TexInfo* texInfos = new Texpack::TexInfo[_TexsCount];
    Texpack::BlockInfo* blockInfos = new Texpack::BlockInfo[_TexsCount];

    size_t woff = _texSectionOff;
    for (size_t i = 0; i < _TexsCount; i++)
    {
        Gnf::GnfImage*& gnfImg = gnfImages[i];

        for (size_t j = 0; j < texpacks.size(); j++)
        {
            if (texpacks[j]->GetUserHash(gnfHashes[i], gnfImg->header.userHash))
                break;
        }
        Texpack::TexInfo& texInfo = texInfos[i];
        texInfo._fileHash = gnfHashes[i];
        texInfo._userHash = gnfImg->header.userHash;
        texInfo._blockInfoOff = _blocksInfoOff + (i * 0x20);

        Texpack::BlockInfo& blockInfo = blockInfos[i];
        blockInfo._blockOff = uint32_t(woff >> 4);
        blockInfo._rawSize = gnfImg->header.dataSize;
        blockInfo._blockSize = (gnfImg->header.fileSize + 0x24 + 15) & (~15);
        blockInfo._mipLvlStart = gnfImg->header.mipmaps;
        blockInfo._mipLvlEnd = 0;
        blockInfo._tocFileIdx = 0;
        blockInfo._mipWidth = gnfImg->header.width + 1;
        blockInfo._mipHeight = gnfImg->header.height + 1;
        blockInfo._nextSiblingBlockInfoOff = -1LL;

        woff += blockInfo._blockSize;
    }

    for (size_t i = 0; i < _TexsCount; i++)
    {
        Texpack::TexInfo& texInfo = texInfos[i];

        ofs.write((char*)&texInfo, sizeof(texInfo));
        ofs1.write((char*)&texInfo, sizeof(texInfo));
    }
    for (size_t i = 0; i < _TexsCount; i++)
    {
        Texpack::BlockInfo& blockInfo = blockInfos[i];

        ofs.write((char*)&blockInfo, sizeof(blockInfo));
        ofs1.write((char*)&blockInfo, sizeof(blockInfo));
    }

    size_t curOff = ofs.tellp();
    for (size_t i = 0; i < _texSectionOff - curOff; i++)
    {
        byte zz = 0;
        ofs.write((char*)&zz, sizeof(zz));
        ofs1.write((char*)&zz, sizeof(zz));
    }

    ofs1.close();
    for (size_t i = 0; i < _TexsCount; i++)
    {
        Gnf::GnfImage*& gnfImg = gnfImages[i];
        uint32_t zz = 0x1U;
        ofs.write((char*)&zz, sizeof(zz));
        zz = 0x124U;
        ofs.write((char*)&zz, sizeof(zz));

        uint32_t blockSize = (gnfImg->header.fileSize + 0x24 + 15) & (~15);
        ofs.write((char*)&blockSize, sizeof(blockSize));
        zz = 0x5U;
        ofs.write((char*)&zz, sizeof(zz));
        ofs.write((char*)&gnfImg->header, sizeof(gnfImg->header));

        uint64_t zzz = 0x3ULL;
        ofs.write((char*)&zzz, sizeof(zzz));
        uint16_t zzzz = uint16_t(gnfImg->header.mipmaps + 1);
        ofs.write((char*)&zzzz, sizeof(zzzz));
        ofs.write((char*)&zzzz, sizeof(zzzz));
        ofs.write((char*)&gnfImg->header.dataSize, sizeof(gnfImg->header.dataSize));
        zz = 0x2U;
        ofs.write((char*)&zz, sizeof(zz));

        ofs.write((char*)gnfImg->imageData.get(), gnfImg->header.dataSize);

        curOff = ofs.tellp();
        for (size_t j = 0; j < ((curOff + 15) & (~15)) - curOff; j++)
        {
            byte z = 0;
            ofs.write((char*)&z, sizeof(z));
        }
    }
    ofs.close();
    return true;
}
bool ExportAllTextures(WADArchive& wad, vector<Texpack*>& texpacks, const std::filesystem::path& outdir,bool dds)
{
    if (wad._fileEntries.size() < 1 || texpacks.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    for (int i = 0; i < wad._fileEntries.size(); i++)
    {
        if (wad._fileEntries[i].type == WADArchive::FileType::GOWR_TEXTURE)
        {
            std::stringstream s;
            std::string name = wad._fileEntries[i].nameStr().substr(3);
            s << std::hex << name.substr(name.find_last_of("_") + 1, 16);
            uint64_t hash = 0;
            s >> hash;
            for (int j = 0; j < texpacks.size(); j++)
            {
                if (texpacks[j]->ContainsTexture(hash))
                {
                    texpacks[j]->ExportGnf(outdir, hash, wad._fileEntries[i].nameStr(), dds);
                    break;
                }
            }
        }
    }
    return true;
}
bool ExportAllSkinnedMesh(WADArchive& wad, vector<Lodpack*>& lodpacks,const std::filesystem::path& outdir, bool Lods = false)
{
    if (wad._header.fileCount < 1 || lodpacks.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    vector<int> usedMeshBufferIndices;
    vector<int> usedMgDefIndices;
    for (int i = 0; i < wad._header.fileCount; i++)
    {
        if (wad._fileEntries[i].type == WADArchive::FileType::GOWR_MESH_DEFN && wad._fileEntries[i].nameStr().substr(0,4) == "MESH")
        {

            std::string name = wad._fileEntries[i].nameStr().substr(5);
            std::stringstream meshDefStream;
            std::stringstream mgDefStream;
            std::stringstream meshBuffStream;
            std::stringstream rigStream;
            wad.GetFile(i, meshDefStream);
            for (int j = 0; j < wad._header.fileCount; j++)
            {
                if (wad._fileEntries[j].type == WADArchive::FileType::GOWR_MG_GPU_BUFF && (wad._fileEntries[j].nameStr().find(name) != std::string::npos))
                {
                    if (std::find(usedMeshBufferIndices.begin(), usedMeshBufferIndices.end(), j) != usedMeshBufferIndices.end())
                        continue;
                    wad.GetFile(j, meshBuffStream);
                    usedMeshBufferIndices.push_back(j);
                    break;
                }
            }

            bool rigPresent = false;
            Rig rig;

            for (int j = 0; j < wad._header.fileCount; j++)
            {
                if (wad._fileEntries[j].type == WADArchive::FileType::GOWR_GOPROTO_RIG && (wad._fileEntries[j].nameStr().find("Proto") != std::string::npos))
                {
                    rigPresent = true;
                    std::string sample = Utils::str_tolower(wad._fileEntries[j].nameStr().substr(7, wad._fileEntries[j].nameStr().length() - 7));
                    if (sample == name.substr(0,name.length() - 2))
                    {
                        wad.GetFile(j, rigStream);
                        rig = Rig(rigStream);
                        break;
                    }
                }
            }

            if (rigPresent)
            {
                if (wad._fileEntries[i + 2].type == WADArchive::FileType::GOWR_MG_DEFN)
                    wad.GetFile(i + 2, mgDefStream);
                else
                    throw std::exception("test");
                //for (int j = 0; j < wad._header.fileCount; j++)
                //{
                //    if (wad._fileEntries[j].type == WADArchive::FileType::GOWR_MG_DEFN && (wad._fileEntries[j].nameStr().find(name) != std::string::npos))
                //    {
                //        cout << i << " " << j << "\n";
                //        if (std::find(usedMgDefIndices.begin(), usedMgDefIndices.end(), j) != usedMgDefIndices.end())
                //            continue;
                //        wad.GetFile(j, mgDefStream);
                //        usedMgDefIndices.push_back(j);
                //        break;
                //    }
                //}
            }


            vector<MeshInfo> meshInfos;

            meshDefStream.seekg(0, ios::end);
            GOWR::MESH::Parse(meshDefStream, meshInfos, meshDefStream.tellg());

            if (rigPresent)
            {
                mgDefStream.seekg(0, ios::end);
                GOWR::MG::Parse(mgDefStream, meshInfos, mgDefStream.tellg());
            }

            vector<RawMeshContainer> meshes;
            for (int j = 0; j < meshInfos.size(); j++)
            {
                char buf[10];
                sprintf_s(buf, "%04d", j);
                string subname = "submesh_" + string(buf) + "_" + std::to_string(meshInfos[j].LODlvl);
                if (meshInfos[j].LODlvl > 0 && !Lods)
                    continue;
                if (meshInfos[j].Hash == 0)
                {
                    if (meshBuffStream.tellp() != std::streampos(0))
                    {
                        meshes.push_back(containRawMesh(meshInfos[j], meshBuffStream, subname));
                    }
                }
                else
                {
                    std::stringstream buffer;
                    for (int k = 0; k < lodpacks.size(); k++)
                    {
                        if (lodpacks[k]->GetBuffer(meshInfos[j].Hash, buffer))
                            break;
                    }
                    if (buffer.tellp() != std::streampos(0))
                    {
                        meshes.push_back(containRawMesh(meshInfos[j], buffer, subname));
                    }
                }
            }
            if (rigPresent)
            {
                for (auto& mesh : meshes)
                {
                    for (auto& vert : mesh.Joints)
                    {
                        for (auto& x : vert)
                        {
                            x = std::clamp<uint16_t>(x, 0, rig.boneCount - 1);
                        }
                    }
                }
            }
            std::filesystem::path outfile = outdir / (wad._fileEntries[i].nameStr() + "---" + std::to_string(i) + ".glb");
            WriteGLTF(outfile, meshes, rig);
        }
    }
    return true;
}
bool ExportAllRigidMesh(WADArchive& wad, vector<Lodpack*>& lodpacks, const std::filesystem::path& outdir)
{
    if (wad._header.fileCount < 1 || lodpacks.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    vector<int> usedMeshBufferIndices;
    vector<int> usedMgDefIndices;
    for (int i = 0; i < wad._header.fileCount; i++)
    {
        if (wad._fileEntries[i].type == WADArchive::FileType::GOWR_MESH_DEFN && wad._fileEntries[i].nameStr().substr(0, 4) == "MESH")
        {
            if (wad._fileEntries[i].nameStr() != "MESH_houseframe_grp_0")
                continue;
            std::string name = wad._fileEntries[i].nameStr().substr(5);
            std::stringstream meshDefStream;
            std::stringstream meshBuffStream;
            std::stringstream rigStream;
            wad.GetFile(i, meshDefStream);
            for (int j = 0; j < wad._header.fileCount; j++)
            {
                if (wad._fileEntries[j].type == WADArchive::FileType::GOWR_MG_GPU_BUFF && (wad._fileEntries[j].nameStr().find(name) != std::string::npos))
                {
                    if (std::find(usedMeshBufferIndices.begin(), usedMeshBufferIndices.end(), j) != usedMeshBufferIndices.end())
                        continue;
                    wad.GetFile(j, meshBuffStream);
                    usedMeshBufferIndices.push_back(j);
                    break;
                }
            }

            Rig rig;

            vector<MeshInfo> meshInfos;

            meshDefStream.seekg(0, ios::end);
            GOWR::MESH::Parse(meshDefStream, meshInfos, meshDefStream.tellg());

            vector<RawMeshContainer> meshes;
            for (int j = 0; j < meshInfos.size(); j++)
            {
                char buf[10];
                sprintf_s(buf, "%04d", j);
                string subname = "submesh_" + string(buf) + "_" + std::to_string(meshInfos[j].LODlvl);

                if (meshInfos[j].Hash == 0)
                {
                    if (meshBuffStream.tellp() != std::streampos(0))
                    {
                        meshes.push_back(containRawMesh(meshInfos[j], meshBuffStream, subname));
                    }
                }
                else
                {
                    std::stringstream buffer;
                    for (int k = 0; k < lodpacks.size(); k++)
                    {
                        if (lodpacks[k]->GetBuffer(meshInfos[j].Hash, buffer))
                            break;
                    }
                    if (buffer.tellp() != std::streampos(0))
                    {
                        meshes.push_back(containRawMesh(meshInfos[j], buffer, subname));
                    }
                }
            }
            std::filesystem::path outfile = outdir / (wad._fileEntries[i].nameStr() + "---" + std::to_string(i) + ".glb");
            WriteGLTF(outfile, meshes, rig);
        }
    }
    return true;
}
CommandLine Init()
{
    CommandLine cmmdParse("God of War Tool", "Usage:\n  GOWTool");
    cmmdParse.AddCommand("wad", "Target a Wad file for E/I.");
    cmmdParse.AddOption("wad", "-p", "Input path to .wad file.", CommandLine::OptionType::MultiArgs);
    cmmdParse.AddOption("wad", "-u", "Unpack WAD files.");
    cmmdParse.AddOption("wad", "-m", "Export Meshes.");
    cmmdParse.AddOption("wad", "-t", "Export Textures.");

    cmmdParse.AddCommand("settings", "Change Tool Settings.");
    cmmdParse.AddOption("settings", "-g", "Input path to GameDir", CommandLine::OptionType::SingleArg);

    //cmmdParse.AddCommand("texpack", "Target a Texpack file for E/I.");
    //cmmdParse.AddOption("texpack", "-e", "Export Textures from Texpack.");
    //cmmdParse.AddOption("texpack", "-p", "Input path to .texpack file.", CommandLine::OptionType::MultiArgs);


    return cmmdParse;
}
int main(int argc, char* argv[])
{
    //directory_iterator dir(R"(D:\gowr\Image0\exec\wad\orbis_le)");

    //std::fstream file(R"(D:\gowr\test.txt)", std::ios::out);

    //for (auto& itr : dir)
    //{
    //    if (itr.path().extension().string() == ".wad")
    //    {
    //        WADArchive wad;
    //        shared_ptr<fstream> fsptr = make_shared<fstream>(itr.path(), std::ios::binary | std::ios::in);
    //        if (wad.Read(fsptr) && wad.Test())
    //        {
    //            for (int i = 0; i < wad._header.fileCount; i++)
    //            {
    //                //if (wad._fileEntries[i].type == WADArchive::FileType::GOWR_MESH_DEFN && wad._fileEntries[i].nameStr().substr(0, 4) == "MESH")
    //                //{
    //                //    std::stringstream meshDefStream;
    //                //    wad.GetFile(i, meshDefStream);

    //                //    vector<MeshInfo> meshInfos;
    //                //    meshDefStream.seekg(0, ios::end);
    //                //    if (!GOWR::MESH::Parse(meshDefStream, meshInfos, meshDefStream.tellg()))
    //                //    {
    //                //        cout << "\n";
    //                //    }
    //                //}
    //                if (wad._fileEntries[i].type == WADArchive::FileType::GOWR_TEXTURE)
    //                {
    //                    file << wad._fileEntries[i].nameStr() << itr.path().filename() << "\n";
    //                }
    //            }
    //        }
    //    }
    //}

    //return 0;
    //for (auto itr = compMap.begin(); itr != compMap.end(); itr++)
    //{
    //    cout << "Primitive: " << int(itr->first);
    //    cout << " DataTypes:";
    //    for (auto itr1 = itr->second.begin(); itr1 != itr->second.end(); itr1++)
    //    {
    //        cout << " " << int(itr1->first) << " " << itr1->second;
    //    }
    //    cout << "\n";
    //}
    //cout << "================\n";
    //cout << " DataTypes:";
    //for (auto itr = dataSet.begin(); itr != dataSet.end(); itr++)
    //{
    //    cout << " " << int(*itr);
    //}
    //return 0;


    //directory_iterator dir(R"(D:\Game\God of War Ragnarok\exec\wad\pc_le)");

    //std::set<std::tuple<Gnf::Format, Gnf::FormatType>> types;
    //for (auto& itr : dir)
    //{
    //    if (itr.path().extension().string() == ".texpack")
    //    {
    //        Texpack pack(itr.path().string());
    //        pack.SurveyFormats(types);
    //    }
    //}

    CHAR charbuffer[260] = { 0 };
    GetCurrentDirectoryA(sizeof(charbuffer), charbuffer);
    std::filesystem::path configpath = std::filesystem::path(charbuffer) / "config.ini";

    GetPrivateProfileStringA("Settings", "Gamedir", "", charbuffer, sizeof(charbuffer), configpath.string().c_str());
    std::filesystem::path gamedir(charbuffer);

    gamedir = gamedir / R"(exec\wad\pc_le)";

    CommandLine cmmdParse = Init();

    if (cmmdParse.Parse(argc, argv))
    {
        if (cmmdParse._commands["wad"].active)
        {
            auto& cmmd = cmmdParse._commands["wad"];

            if (!cmmd._options["-m"].active && !cmmd._options["-u"].active && !cmmd._options["-t"].active)
            {
                Utils::Logger::Error("Task not specified\n");
                cmmdParse.PrintHelp();
                return -1;
            }

            if (cmmd._options["-m"].active || cmmd._options["-t"].active)
            {
                if (gamedir.empty() || !gamedir.is_absolute() || !std::filesystem::exists(gamedir) || !std::filesystem::is_directory(gamedir))
                {
                    string msg = "Invalid Dir: " + gamedir.string() + "\n";
                    Utils::Logger::Error(msg.c_str());
                    Utils::Logger::Error("GameDir\\exec\\wad\\pc_le is required to obtain texpack / lodpack files\n");
                    Utils::Logger::Error("GameDir (directory containing eboot.bin), pls change it through settings or edit config.ini\n");
                    cmmdParse._commands["wad"].active = false;
                    cmmdParse._commands["settings"].active = true;
                    cmmdParse.PrintHelp();
                    return -1;
                }
            }

            std::vector<Lodpack*> lodpacks;
            if (cmmd._options["-m"].active)
            {
                std::filesystem::directory_iterator dir(gamedir);
                for (const std::filesystem::directory_entry& entry : dir)
                {
                    if (entry.path().extension().string() == ".lodpack")
                    {
                        Lodpack* pack = new Lodpack();
                        pack->Read(entry.path().string());
                        lodpacks.push_back(pack);
                    }
                }
            }

            std::vector<Texpack*> texpacks;
            if (cmmd._options["-t"].active)
            {
                std::filesystem::directory_iterator dir(gamedir);
                for (const std::filesystem::directory_entry& entry : dir)
                {
                    if (entry.path().extension().string() == ".texpack")
                    {
                        Texpack* pack = new Texpack(entry.path().string());
                        texpacks.push_back(pack);
                    }
                }
            }

            auto& opnP = cmmd._options["-p"];
            for (auto itr = opnP.args.begin(); itr != opnP.args.end(); itr++)
            {
                path wadPath = path(*itr);
                if (wadPath.empty() || !wadPath.is_absolute() || !std::filesystem::exists(wadPath) || !std::filesystem::is_regular_file(wadPath) || wadPath.extension().string() != ".wad")
                {
                    string msg = "Invalid/Unspecified .wad file path: " + wadPath.string() + "\n";
                    Utils::Logger::Error(msg.c_str());
                    cmmdParse.PrintHelp();
                    return -1;
                }
                path outDir = wadPath.parent_path() / wadPath.stem();
                WADArchive wad;
                shared_ptr<fstream> fsptr = make_shared<fstream>(wadPath, std::ios::binary | std::ios::in);

                auto ssptr = DecompressWad(fsptr);
                wad.Read(ssptr);

                if (!wad.Test())
                {
                    string msg = "Assertion Failed, Corrupted File/Bad Parsing Logic, file: " + wadPath.string() + "\n";
                    Utils::Logger::Error(msg.c_str());
                    continue;
                }

                if(!std::filesystem::exists(outDir))
                    std::filesystem::create_directory(outDir);

                if (cmmd._options["-u"].active)
                {
                    if (wad.UnpackFiles(outDir.string()))
                    {
                        string msg = "Successfully unpacked all files to: " + outDir.string() + "\n";
                        Utils::Logger::Success(msg.c_str());
                    }
                    else
                    {
                        string msg = "Failed to unpacked all files from: " + wadPath.string() + "\n";
                        Utils::Logger::Error(msg.c_str());
                    }
                }
                if (cmmd._options["-t"].active)
                {
                    if (ExportAllTextures(wad, texpacks, outDir, true))
                    {
                        string msg = "Successfully exported textures to: " + outDir.string() + "\n";
                        Utils::Logger::Success(msg.c_str());
                    }
                    else
                    {
                        string msg = "Failed to export textures from: " + wadPath.string() + "\n";
                        Utils::Logger::Error(msg.c_str());
                    }
                }
                if (cmmd._options["-m"].active)
                {
                    if (ExportAllSkinnedMesh(wad, lodpacks, outDir))
                    {
                        string msg = "Successfully exported meshes to: " + outDir.string() + "\n";
                        Utils::Logger::Success(msg.c_str());
                    }
                    else
                    {
                        string msg = "Failed to export meshes files from: " + wadPath.string() + "\n";
                        Utils::Logger::Error(msg.c_str());
                    }
                }
            }
        }
        if (cmmdParse._commands["texpack"].active)
        {
            auto& cmmd = cmmdParse._commands["texpack"];

            auto& opnP = cmmd._options["-p"];

            if (!cmmd._options["-e"].active)
            {
                Utils::Logger::Error("Export Task not specified\n");
                cmmdParse.PrintHelp();
                return -1;
            }

            for (auto itr = opnP.args.begin(); itr != opnP.args.end(); itr++)
            {
                path texPath = path(*itr);
                if (texPath.empty() || !texPath.is_absolute() || !std::filesystem::exists(texPath) || !std::filesystem::is_regular_file(texPath) || texPath.extension().string() != ".texpack")
                {
                    string msg = "Invalid/Unspecified .texpack file path: " + texPath.string() + "\n";
                    Utils::Logger::Error(msg.c_str());
                    cmmdParse.PrintHelp();
                    return -1;
                }
                path outDir = texPath.parent_path() / texPath.stem();

                if (!std::filesystem::exists(outDir))
                    std::filesystem::create_directory(outDir);

                if (cmmd._options["-e"].active)
                {
                    Texpack pack = Texpack(texPath.string());
                    if (pack.ExportAllGnf(outDir, false))
                    {
                        Utils::Logger::Success(("Successfully exported all textures to: " + outDir.string() + "\n").c_str());
                    }
                    else
                    {
                        Utils::Logger::Error(("Failed To Export Textures From: " + texPath.string() + "\n").c_str());
                    }
                }
                else
                {
                    Utils::Logger::Error("Export Task not specified\n");
                    cmmdParse.PrintHelp();
                    return -1;
                }
            }
        }
        if (cmmdParse._commands["settings"].active)
        {
            auto& cmmd = cmmdParse._commands["settings"];

            auto& opnG = cmmd._options["-g"];

            if (!cmmd._options["-g"].active)
            {
                Utils::Logger::Error("Export Task not specified\n");
                cmmdParse.PrintHelp();
                return -1;
            }

            gamedir = std::filesystem::path(opnG.args[0]);

            if (gamedir.empty() || !gamedir.is_absolute() || !std::filesystem::exists(gamedir) || !std::filesystem::is_directory(gamedir))
            {
                Utils::Logger::Error(("Invalid gamedir specified: " + gamedir.string() + "\n").c_str());
                cmmdParse.PrintHelp();
                return -1;
            }
            if (WritePrivateProfileStringA("Settings", "Gamedir", gamedir.string().c_str(), configpath.string().c_str()))
            {
                Utils::Logger::Success("Gamedir Updated\n");
                return 0;
            }
        }
    }
    else
        cmmdParse.PrintHelp();
} 