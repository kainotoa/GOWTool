#include "pch.h"
#include "MainFunctions.h"
#include <filesystem>
#include "glTFSerializer.h"
int main(void)
{
    string lod = R"(D:\God.of.War.4.Latino\CUSA07408\exec\wad\orbis_le\root.lodpack)";
    string m = R"(D:\r_heroa00\1\MG_heroa00_0.dat)";
    string g = R"(D:\r_heroa00\29\MG_heroa00_0_gpu.dat)";

    ifstream gStream(g, std::ios::in | std::ios::binary);
    ifstream lodStream(lod, std::ios::in | std::ios::binary);


    Lodpack lodpack;
    lodpack.ReadLodpack(lod);

    MGDefinition mgdef;
    mgdef.ReadMG(m);

    auto infos = getMeshesInfo(mgdef, m);
    vector<RawMesh> meshes;
    for (int i = 0; i < infos.size(); i++)
    {
        if (infos[i].LODlvl > 0)
            continue;
        if (infos[i].Hash == 0)
            meshes.push_back(containRawMesh(infos[i], gStream));
        else
        {
            for (int e = 0; e < lodpack.TotalmembersCount; e++)
            {
                if (lodpack.memberHash[e] == infos[i].Hash)
                {
                    meshes.push_back(containRawMesh(infos[i], lodStream, lodpack.memberOffsetter[e] + lodpack.groupStartOff[lodpack.memberGroupIndex[e]]));
                }
            }

        }
    }
    WriteGLTF(std::filesystem::path(R"(D:\test.glb)"), meshes);
    gStream.close();
    lodStream.close();
}