#include "pch.h"
#include "Rig.h"
#include "glTFSerializer.h"
#include "Formats.h"
#include "MainFunctions.h"
#include  "Texpack.h"
int main(void)
{
    /*
    for (auto& p : std::filesystem::directory_iterator(R"(D:\tetet\New folder\1)"))
    {
        if (p.path().string().find("MG_") != std::string::npos)
        {
            cout << p.path().string() << "\n";
            MGDefinition mg;
            mg.ReadMG(p.path().string());
        }
    }
    */
    
    string lod = R"(D:\God.of.War.4.Latino\CUSA07408\exec\wad\orbis_le\root.lodpack)";
    string m = R"(D:\tetet\New folder\1\MG_freya00_0.dat)";
    string g = R"(D:\tetet\New folder\29\MG_freya00_0_gpu.dat)";

    ifstream gStream(g, std::ios::in | std::ios::binary);
    ifstream lodStream(lod, std::ios::in | std::ios::binary);


    Lodpack lodpack;
    lodpack.ReadLodpack(lod);

    MGDefinition mgdef;


    auto infos = mgdef.ReadMG(m);
    vector<RawMeshContainer> meshes;
    int count = 0;
    for (size_t i = 0; i < infos.size(); i++)
    {
        if (infos[i].LODlvl > 0)
            continue;
        if (infos[i].Hash == 0)
            meshes.push_back(containRawMesh(infos[i], gStream));
        else
        {
            for (uint32_t e = 0; e < lodpack.TotalmembersCount; e++)
            {
                if (lodpack.memberHash[e] == infos[i].Hash)
                {
                    meshes.push_back(containRawMesh(infos[i], lodStream, lodpack.memberOffsetter[e] + lodpack.groupStartOff[lodpack.memberGroupIndex[e]]));
                }
            }

        }
        count++;
    }
    //Rig rig;
    Rig rig(R"(D:\tetet\New folder\1\goProtofreya00.dat)");
    WriteGLTF(std::filesystem::path(R"(D:\test.glb)"), meshes, rig);
    gStream.close();
    lodStream.close();
    
} 