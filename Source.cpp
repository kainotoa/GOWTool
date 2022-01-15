#include "pch.h"
#include "Rig.h"
#include "glTFSerializer.h"
#include "Formats.h"
#include "MainFunctions.h"
#include "Texpack.h"
#include "Lodpak.h"
#include "Wad.h"
#include "krak.h"
#include "Gnf.h"
#include "utils.h"

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );
    return s;
}
bool ExportAllTextures(WadFile& wad, vector<Texpack*>& texpacks, const std::filesystem::path& outdir)
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
                    texpacks[j]->ExportGnf(outdir, hash, wad._FileEntries[i].name);
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
                    std::string sample = str_tolower(wad._FileEntries[j].name.substr(7, wad._FileEntries[j].name.length() - 7));
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
            if (ExportAllTextures(wad, texpacks, outdir))
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
            cout << "  -p, --path <path>        Input path to .texpack file.\n";
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
            else
            {
                Utils::Logger::Error(("Invalid option or argument: " + op).c_str());
                LogHelp();
                return -1;
            }
        }
        if (path.empty() || !path.is_absolute() || !std::filesystem::exists(path) || !std::filesystem::is_regular_file(path) || path.extension().string() != ".texpack")
        {
            Utils::Logger::Error(("\nInvalid/Unspecified .texpack file path: " + path.string()).c_str());
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

        Texpack pack = Texpack(path.string());
        if (pack.ExportAllGnf(outdir))
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
    else if (command == "settings")
    {
        auto LogHelp = []()
        {
            cout << "\nsettings\n";
            cout << "  Change tool settings.\n";
            cout << "\nUsage:\n";
            cout << "  GOWTool settings [options]\n";
            cout << "\nOptions:\n";
            cout << "  -g, --gamedir <gamedir>  Input path to the God of War PS4 gamefiles directory\n";
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