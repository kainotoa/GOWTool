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
#include <chrono>

int main(void)
{
    LoadLib();
    auto y =  std::chrono::high_resolution_clock::now();
    std::filesystem::path p(R"(E:\God.of.War.4.Latino\CUSA07408\exec\wad\orbis_le\root.texpack)");
    Texpack tex(p);
    tex.ExportAllGnf(std::filesystem::path(R"(D:\new test\)"));
    auto z = std::chrono::high_resolution_clock::now();
    auto res = std::chrono::duration_cast<std::chrono::seconds>(z - y);
    cout << res.count();
    /*
    Gnf::Header header;
    std::ifstream fs(R"(E:\test\6034574368530819.gnf)",std::ios::in | std::ios::binary);
    fs.seekg(0, std::ios::beg);
    fs.read((char*)&header, sizeof(header));
    fs.close();
    */
    //Texpack tex(R"(E:\God.of.War.4.Latino\CUSA07408\exec\wad\orbis_le\root.texpack)");
    //tex.ExportAll(R"(E:\test)");
    /*
    WADFile wad(R"(D:\God.of.War.4.Latino\CUSA07408\exec\wad\orbis_le\r_athena00.wad)");
    
    string lod = R"(D:\God.of.War.4.Latino\CUSA07408\exec\wad\orbis_le\root.lodpack)";
    Lodpack lodpack(lod);

    std::stringstream s;
    std::stringstream ss;
    wad.GetBuffer(96, s);
    wad.GetBuffer(95, ss);
    MGDefinition mg;

    auto infos = mg.ReadMG(s);
    vector<RawMeshContainer> meshes;
    std::stringstream sss;
    for (size_t i = 0; i < infos.size(); i++)
    {
        if (infos[i].LODlvl > 0)
            continue;
        if (infos[i].Hash == 0)
            meshes.push_back(containRawMesh(infos[i], ss));
        else
        {
            lodpack.GetBuffer(infos[i].Hash, sss);
            meshes.push_back(containRawMesh(infos[i], sss));
        }
    }
    std::stringstream ssss;
    wad.GetBuffer(104, ssss);
    Rig rig(ssss);
    WriteGLTF(std::filesystem::path(R"(D:\test.gltf)"), meshes, rig);
    cout << "";
    */
} 