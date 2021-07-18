#include "pch.h"
#include "Rig.h"
#include "glTFSerializer.h"
#include "Formats.h"
#include "MainFunctions.h"
#include "Texpack.h"
#include "Lodpak.h"
#include "Wad.h"
int main(void)
{
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
} 