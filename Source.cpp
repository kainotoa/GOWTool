#include "pch.h"
#include "Rig.h"
#include "glTFSerializer.h"
#include "Formats.h"
#include "MainFunctions.h"
#include "Texpack.h"
#include "Lodpak.h"
#include "Wad.h"
#include "krak.h"
#include "utils.h"
#include "Gnf.h"
#include "converter.h"

bool ImportAllGnf(const std::filesystem::path& gnfSrcDir, vector<Texpack*>& texpacks)
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
bool ExportAllTextures(WadFile& wad, vector<Texpack*>& texpacks, const std::filesystem::path& outdir,bool dds)
{
    if (wad._FileEntries.size() < 1 || texpacks.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    for (int i = 0; i < wad._FileEntries.size(); i++)
    {
        if (wad._FileEntries[i].type == WadFile::FileType::Texture)
        {
            std::stringstream s;
            std::string name = wad._FileEntries[i].name.substr(3);
            s << std::hex << name.substr(name.find_last_of("_") + 1, 16);
            uint64_t hash = 0;
            s >> hash;
            for (int j = 0; j < texpacks.size(); j++)
            {
                if (texpacks[j]->ContainsTexture(hash))
                {
                    texpacks[j]->ExportGnf(outdir, hash, wad._FileEntries[i].name, dds);
                    break;
                }
            }
        }
    }
    return true;
}
bool ExtractAllFiles(WadFile& wad, const std::filesystem::path& outdir)
{
    if (wad._FileEntries.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    for (int i = 0; i < wad._FileEntries.size(); i++)
    {
        std::filesystem::path outfile = outdir / (wad._FileEntries[i].name + "." + std::to_string(i) + ".bin");
        std::fstream fs;
        fs.open(outfile.string(), ios::binary | ios::out);
        wad.GetBuffer(i, fs);
        fs.close();
    }
    return true;
}
bool ExportAllSkinnedMesh(WadFile& wad, vector<Lodpack*>& lodpacks,const std::filesystem::path& outdir)
{
    if (wad._FileEntries.size() < 1 || lodpacks.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    vector<int> usedMeshBufferIndices;
    for (int i = 0; i < wad._FileEntries.size(); i++)
    {
        if (wad._FileEntries[i].type == WadFile::FileType::SkinnedMeshDef)
        {
            std::string name = wad._FileEntries[i].name.substr(3, wad._FileEntries[i].name.length() - 5);
            std::stringstream meshDefStream;
            std::stringstream meshBuffStream;
            std::stringstream rigStream;
            wad.GetBuffer(i, meshDefStream);
            for (int j = 0; j < wad._FileEntries.size(); j++)
            {
                if (wad._FileEntries[j].type == WadFile::FileType::SkinnedMeshBuff && (wad._FileEntries[j].name.find(wad._FileEntries[i].name) != std::string::npos))
                {
                    if (std::find(usedMeshBufferIndices.begin(), usedMeshBufferIndices.end(), j) != usedMeshBufferIndices.end())
                        continue;
                    wad.GetBuffer(j, meshBuffStream);
                    usedMeshBufferIndices.push_back(j);
                    break;
                }
            }
            for (int j = 0; j < wad._FileEntries.size(); j++)
            {
                if (wad._FileEntries[j].type == WadFile::FileType::Rig && (wad._FileEntries[j].name.find("Proto") != std::string::npos))
                {
                    std::string sample = Utils::str_tolower(wad._FileEntries[j].name.substr(7, wad._FileEntries[j].name.length() - 7));
                    if (sample == name)
                    {
                        wad.GetBuffer(j, rigStream);
                        break;
                    }
                }
            }

            MGDefinition meshDef;
            auto meshInfos = meshDef.ReadMG(meshDefStream);
            Rig rig(rigStream);

            vector<RawMeshContainer> meshes;
            for (int j = 0; j < meshInfos.size(); j++)
            {
                char buf[10];
                sprintf_s(buf, "%04d", j);
                string subname = "submesh_" + string(buf) + "_" + std::to_string(meshInfos[j].LODlvl);
                if (meshInfos[j].LODlvl > 0)
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
            std::filesystem::path outfile = outdir / (wad._FileEntries[i].name + "." + std::to_string(i) + ".glb");
            WriteGLTF(outfile, meshes, rig);
        }
    }
    return true;
}
bool ExportAllRigidMesh(WadFile& wad, vector<Lodpack*>& lodpacks, const std::filesystem::path& outdir)
{
    if (wad._FileEntries.size() < 1 || lodpacks.size() < 1)
        return false;
    if (!std::filesystem::exists(outdir))
        return false;
    vector<int> usedMeshBufferIndices;
    for (int i = 0; i < wad._FileEntries.size(); i++)
    {
        if (wad._FileEntries[i].type == WadFile::FileType::RigidMeshDefData && (wad._FileEntries[i].name.find("smsh_data") != std::string::npos))
        {
            std::string name = wad._FileEntries[i].name.substr(0, wad._FileEntries[i].name.length() - 10);
            std::stringstream meshDefStream;
            wad.GetBuffer(i, meshDefStream);
            SmshDefinition smshDef;
            auto meshInfos = smshDef.ReadSmsh(meshDefStream);

            std::stringstream meshBuffStream;
            for (int j = 0; j < wad._FileEntries.size(); j++)
            {
                if (wad._FileEntries[j].type == WadFile::FileType::SkinnedMeshBuff && (wad._FileEntries[j].name.find(name) != std::string::npos))
                {
                    if (std::find(usedMeshBufferIndices.begin(), usedMeshBufferIndices.end(), j) != usedMeshBufferIndices.end())
                        continue;
                    wad.GetBuffer(j, meshBuffStream);
                    usedMeshBufferIndices.push_back(j);
                    break;
                }
            }
            Rig rig;

            vector<RawMeshContainer> meshes;
            for (int j = 0; j < meshInfos.size(); j++)
            {
                char buf[10];
                sprintf_s(buf, "%04d", j);
                string subname = "submesh_" + string(buf) + "_" + std::to_string(meshInfos[j].LODlvl);
                if (meshInfos[j].LODlvl > 0)
                    continue;
                if (meshInfos[j].Hash == 0)
                {
                    meshes.push_back(containRawMesh(meshInfos[j], meshBuffStream, subname));
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
            std::filesystem::path outfile = outdir / (wad._FileEntries[i].name + "." + std::to_string(i) + ".glb");
            WriteGLTF(outfile, meshes, rig);
        }
    }
    return true;
}

void PrintHelp()
{
    cout << "\nGod of War Tool\n";
    cout << "\nUsage:\n";
    cout << "  GOWTool [command] [options]\n";
    cout << "\nCommands:\n";
    cout << "  wad       Target a Wad file for export.\n";
    cout << "  texpack   Target a Texpack file for export.\n";
    cout << "  settings  Change tool settings.\n";
    cout << "\nOptions:\n";
    cout << "  -h, --help  Show help and usage information.\n";
}
int main(int argc, char* argv[])
{
    /*
    for (int i = 0; i < argc; i++)
    {
        printf("arg[%d]: %s\n", i, argv[i]);
    }
    */
    if (argc < 2)
    {
        Utils::Logger::Error("Required argument was not provided.\n");
        PrintHelp();
        return -1;
    }

    CHAR charbuffer[260] = { 0 };
    GetCurrentDirectoryA(sizeof(charbuffer), charbuffer);
    std::filesystem::path configpath = std::filesystem::path(charbuffer) / "config.ini";

    GetPrivateProfileStringA("Settings", "Gamedir", "", charbuffer, sizeof(charbuffer), configpath.string().c_str());
    std::filesystem::path gamedir(charbuffer);

    std::string command(argv[1]);

    if (command == "-h" || command == "--help")
    {
        PrintHelp();
        return 0;
    }
    else if (command == "wad")
    {
        auto LogHelp = []()
        {
            cout << "\nwad\n";
            cout << "  Target a Wad file for export.\n";
            cout << "\nUsage:\n";
            cout << "  GOWTool wad [options]\n";
            cout << "\nOptions:\n";
            cout << "  -p, --path <path>        Input path to .wad file.\n";
            cout << "  -o, --outpath <outpath>  Output directory.\n";
            cout << "  -e, --extract            Extract all files from .wad.\n";
            cout << "  -m, --mesh               Export all meshes from .wad.\n";
            cout << "  -t, --texture            Export all textures from .wad.\n";
            cout << "  -d, --dds                Export Textures in DDS Format.\n";
            cout << "  -h, --help               Show help and usage information.\n";

        };
        if (argc < 3)
        {
            Utils::Logger::Error("No option/arguments provided: ");
            LogHelp();
            return -1;
        }
        std::filesystem::path path;
        std::filesystem::path outdir;
        bool mesh = false;
        bool texture = false;
        bool extract = false;
        bool dds = false;
        for (int i = 2; i < argc; i++)
        {
            std::string op(argv[i]);
            if (op == "-h" || op == "--help")
            {
                LogHelp();
                return 0;
            }
            else if (op == "-p" || op == "--path")
            {
                if (argc > (i + 1))
                {
                    path = std::filesystem::path(argv[i + 1]);
                    i++;
                }
                else
                {
                    Utils::Logger::Error("\nRequired argument missing for option: -p");
                    LogHelp();
                    return -1;
                }
            }
            else if (op == "-o" || op == "--outpath")
            {
                if (argc > (i + 1))
                {
                    outdir = std::filesystem::path(argv[i + 1]);
                    i++;
                }
                else
                {
                    Utils::Logger::Error("\nRequired argument missing for option: -o");
                    LogHelp();
                    return -1;
                }
            }
            else if (op == "-e" || op == "--extract")
            {
                extract = true;
            }
            else if (op == "-m" || op == "--mesh")
            {
                mesh = true;
            }
            else if (op == "-d" || op == "--dds")
            {
                dds = true;
            }
            else if (op == "-t" || op == "--texture")
            {
                texture = true;
            }
            else
            {
                Utils::Logger::Error(("\nInvalid option or argument: " + op).c_str());
                LogHelp();
                return -1;
            }
        }
        if (!mesh && !texture && !extract)
        {
            Utils::Logger::Error("\nExport file type option not specified");
            LogHelp();
            return -1;
        }
        if (path.empty() || !path.is_absolute() || !std::filesystem::exists(path) || !std::filesystem::is_regular_file(path) || path.extension().string() != ".wad")
        {
            Utils::Logger::Error(("\nInvalid/Unspecified .wad file path: " + path.string()).c_str());
            LogHelp();
            return -1;
        }
        if (gamedir.empty() || !gamedir.is_absolute() || !std::filesystem::exists(gamedir) || !std::filesystem::is_directory(gamedir))
        {
            Utils::Logger::Error("\nGame dir is either not specified in config.ini or its invalid, pls change it through settings or edit config.ini");
            LogHelp();
            return -1;
        }
        if (outdir.empty())
        {
            outdir = path.parent_path() / path.stem();
            std::filesystem::create_directory(outdir);
        }
        if(!outdir.is_absolute() || !std::filesystem::exists(outdir) || !std::filesystem::is_directory(outdir))
        {
            Utils::Logger::Error(("\nInvalid outdir specified: " + outdir.string()).c_str());
            LogHelp();
            return -1;
        }
        WadFile wad;
        wad.Read(path);
        if (extract)
        {
            if (ExtractAllFiles(wad, outdir))
            {
                Utils::Logger::Success(("\nSuccessfully extracted all files to: " + outdir.string()).c_str());
            }
            else
            {
                Utils::Logger::Error("\nMeshes export Failed.");
            }
        }
        if (mesh)
        {
            std::filesystem::recursive_directory_iterator dir(gamedir);
            std::vector<Lodpack*> lodpacks;
            for (const std::filesystem::directory_entry& entry : dir)
            {
                if (entry.path().extension().string() == ".lodpack")
                {
                    Lodpack* pack = new Lodpack(entry.path().string());
                    lodpacks.push_back(pack);
                }
            }
            if (lodpacks.size() < 1)
            {
                Utils::Logger::Error("\nspecified gamedir(including sub-directories) doesn't contain any .lodpack files, export failed");
                return -1;
            }
            if (ExportAllSkinnedMesh(wad, lodpacks, outdir) && ExportAllRigidMesh(wad, lodpacks, outdir))
            {
                Utils::Logger::Success(("\nSuccessfully exported all meshes to: " + outdir.string()).c_str());
            }
            else
            {
                Utils::Logger::Error("\nMeshes export Failed.");
            }
        }
        if (texture)
        {
            std::filesystem::recursive_directory_iterator dir(gamedir);
            std::vector<Texpack*> texpacks;
            for (const std::filesystem::directory_entry& entry : dir)
            {
                if (entry.path().extension().string() == ".texpack")
                {
                    Texpack* pack = new Texpack(entry.path().string());
                    texpacks.push_back(pack);
                }
            }
            if (texpacks.size() < 1)
            {
                Utils::Logger::Error("\nspecified gamedir(including sub-directories) doesn't contain any .texpack files, export failed");
                return -1;
            }
            if (ExportAllTextures(wad, texpacks, outdir,dds))
            {
                Utils::Logger::Success(("\nSuccessfully exported all textures to: " + outdir.string()).c_str());
            }
            else
            {
                Utils::Logger::Error("\nTextures export Failed.");
            }
        }
        return 0;
    }
    else if (command == "texpack")
    {
        auto LogHelp = []()
        {
            cout << "\ntexpack\n";
            cout << "  Target a Texpack file for export.\n";
            cout << "\nUsage:\n";
            cout << "  GOWTool texpack [options]\n";
            cout << "\nOptions:\n";
            cout << "  -e, --export             Export textures from .texpack file.\n";
            cout << "  -d, --dds                Export Textures in DDS Format.\n";
            cout << "  -i, --import             Import textures and pack .texpack file.\n";
            cout << "  -p, --path <path>        path to .texpack file or path to directory containing dds/gnf files for import.\n";
            cout << "  -o, --outpath <outpath>  Output directory.\n";
        };
        if (argc < 3)
        {
            Utils::Logger::Error("\nNo option/arguments provided: ");
            LogHelp();
            return -1;
        }
        std::filesystem::path path;
        std::filesystem::path outdir;
        bool imp = false;
        bool exp = false;
        bool dds = false;
        for (int i = 2; i < argc; i++)
        {
            std::string op(argv[i]);
            if (op == "-h" || op == "--help")
            {
                LogHelp();
                return 0;
            }
            else if (op == "-p" || op == "--path")
            {
                if (argc > (i + 1))
                {
                    path = std::filesystem::path(argv[i + 1]);
                    i++;
                }
                else
                {
                    Utils::Logger::Error("Required argument missing for option: -p");
                    LogHelp();
                    return -1;
                }
            }
            else if (op == "-o" || op == "--outpath")
            {
                if (argc > (i + 1))
                {
                    outdir = std::filesystem::path(argv[i + 1]);
                    i++;
                }
                else
                {
                    Utils::Logger::Error("Required argument missing for option: -o");
                    LogHelp();
                    return -1;
                }
            }
            else if (op == "-e" || op == "--export")
            {
                imp = true;
            }
            else if (op == "-i" || op == "--import")
            {
                exp = true;
            }
            else if (op == "-d" || op == "--dds")
            {
                dds = true;
            }
            else
            {
                Utils::Logger::Error(("Invalid option or argument: " + op).c_str());
                LogHelp();
                return -1;
            }
        }
        if (imp && exp)
        {
            Utils::Logger::Error("\nCannot export and import textures at the same time, choose one!");
            LogHelp();
            return -1;
        }
        if (!imp && !exp)
        {
            Utils::Logger::Error("\nTexpack export or import not specified!");
            LogHelp();
            return -1;
        }
        if (outdir.empty())
        {
            outdir = path.parent_path() / path.stem();
            std::filesystem::create_directory(outdir);
        }
        if (!outdir.is_absolute() || !std::filesystem::exists(outdir) || !std::filesystem::is_directory(outdir))
        {
            Utils::Logger::Error(("\nInvalid outdir specified: " + outdir.string()).c_str());
            LogHelp();
            return -1;
        }
        if (exp)
        {
            if (path.empty() || !path.is_absolute() || !std::filesystem::exists(path) || !std::filesystem::is_regular_file(path) || path.extension().string() != ".texpack")
            {
                Utils::Logger::Error(("\nInvalid/Unspecified .texpack file path: " + path.string()).c_str());
                LogHelp();
                return -1;
            }
            Texpack pack = Texpack(path.string());
            if (pack.ExportAllGnf(outdir,dds))
            {
                Utils::Logger::Success(("\nSuccessfully exported all textures to: " + outdir.string()).c_str());
                return 0;
            }
            else
            {
                Utils::Logger::Error("\nTextures export Failed.");
                return -1;
            }
        }
        if (imp)
        {
            if (!path.is_absolute() || !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
            {
                Utils::Logger::Error(("\nInvalid/Unspecified dds/gnf directory " + path.string()).c_str());
                LogHelp();
                return -1;
            }
            std::filesystem::recursive_directory_iterator dir(gamedir);
            std::vector<Texpack*> texpacks;
            for (const std::filesystem::directory_entry& entry : dir)
            {
                if (entry.path().extension().string() == ".texpack")
                {
                    Texpack* pack = new Texpack(entry.path().string());
                    texpacks.push_back(pack);
                }
            }
            if (texpacks.size() < 1)
            {
                Utils::Logger::Error("\nspecified gamedir(including sub-directories) doesn't contain any .texpack files, import failed");
                return -1;
            }
            if (ImportAllGnf(path,texpacks))
            {
                Utils::Logger::Success("\nSuccessfully Imported all and packed textures to .texpack ");
                return 0;
            }
            else
            {
                Utils::Logger::Error("\nTextures export Failed.");
                return -1;
            }
        }
    }
    else if (command == "settings")
    {
        auto LogHelp = []()
        {
            cout << "\nsettings\n";
            cout << "  Change tool settings.\n";
            cout << "\nUsage:\n";
            cout << "  GOWTool settings [options]\n";
            cout << "\nOptions:\n";
            cout << "  -g, --gamedir <gamedir>  Input path to the God of War gamefiles directory\n";
        };
        if (argc < 3)
        {
            Utils::Logger::Error("\nNo option/arguments provided: ");
            LogHelp();
            return -1;
        }
        for (int i = 2; i < argc; i++)
        {
            std::string op(argv[i]);
            if (op == "-h" || op == "--help")
            {
                LogHelp();
                return 0;
            }
            else if (op == "-g" || op == "--gamedir")
            {
                if (argc > (i + 1))
                {
                    gamedir = std::filesystem::path(argv[i + 1]);
                    i++;
                }
                else
                {
                    Utils::Logger::Error("\nRequired argument missing for option: -g");
                    LogHelp();
                    return -1;
                }
            }
            else
            {
                Utils::Logger::Error(("\nInvalid option or argument: " + op).c_str());
                LogHelp();
                return -1;
            }
        }
        if (gamedir.empty() || !gamedir.is_absolute() || !std::filesystem::exists(gamedir) || !std::filesystem::is_directory(gamedir))
        {
            Utils::Logger::Error(("\nInvalid gamedir specified: " + gamedir.string()).c_str());
            LogHelp();
            return -1;
        }
        if (WritePrivateProfileStringA("Settings", "Gamedir", gamedir.string().c_str(), configpath.string().c_str()))
        {
            Utils::Logger::Success("\nGamedir Updated");
            return 0;
        }
    }
    else
    {
        Utils::Logger::Error(("Invalid command or argument: " + command).c_str());
        PrintHelp();
        return -1;
    }

    return 0;
    /*
    CHAR charbuffer[260] = { 0 };
    GetCurrentDirectoryA(sizeof(charbuffer), charbuffer);

    std::filesystem::path configpath = std::filesystem::path(charbuffer) / "config.ini";

    GetPrivateProfileStringA("Settings", "Gamedir", "", charbuffer, sizeof(charbuffer), configpath.string().c_str());
    std::string gamedirstr(charbuffer);
    if (gamedirstr.empty())
    {
        gamedirstr = std::filesystem::path(Utils::FileDialogs::OpenFile("GOW Boot (eboot.bin)\0eboot.bin\0","Open God of War eboot.bin")).parent_path().string();
        if (gamedirstr.empty())
        {
            return -1;
        }
        if (!WritePrivateProfileStringA("Settings", "Gamedir",gamedirstr.c_str(), configpath.string().c_str()))
        {
            return -1;
        }
    }
    */
} 