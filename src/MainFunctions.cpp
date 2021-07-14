#include <pch.h>
#include "MainFunctions.h"
#include "MathFunctions.h"
#include "../inc/Mesh.h"
#include "../inc/Formats.h"

inline uint16_t Switcher(std::vector<MeshComp> meshcomp, uint16_t common, uint16_t looper)
{
    switch (meshcomp[looper].dataType)
    {
    case 0:
    case 2:
    case 3:
        common += meshcomp[looper].elemCount * 4;
        break;
    case 4:
    case 6:
    case 7:
    case 1:
        common += meshcomp[looper].elemCount * 2;
        break;
    case 8:
        common += meshcomp[looper].elemCount * 1;
        break;
    default:
        break;
    }
    return common;
}
inline uint16_t SizeFinder(uint16_t counter, std::vector<MeshComp> meshcomp)
{
    int size = 0;
    size = Switcher(meshcomp, size, counter);
    return size;
}
inline uint32_t OffsetFinder(std::ifstream& uFs, std::vector<MeshComp> meshcomp, uint16_t buffC, uint16_t counter, uint32_t offset, uint32_t vertOffset, uint16_t compCount)
{
    uint16_t offstrider = 0;
    uint32_t returnOffset = 0;
    if (counter <= (compCount - buffC))
    {
        for (int d = 0; d < counter; d++)
        {
            offstrider = Switcher(meshcomp, offstrider, d);
        }
        returnOffset = vertOffset + offstrider;
    }
    else
    {
        uFs.seekg((uint64_t)offset + 4 * ((uint64_t)counter - ((uint64_t)compCount - (uint64_t)buffC)));
        uFs.read((char*)&returnOffset, sizeof(uint32_t));
    }

    return returnOffset;
}
std::vector<MeshInfo> getMeshesInfo(MGDefinition mg,std::string filename)
{
	std::ifstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	std::vector<MeshInfo> meshesInfo;

	for (uint32_t i = 0; i < mg.meshCount; i++)
	{
		MeshInfo tempMesheInfo;
        tempMesheInfo.LODlvl = mg.LODlvl[i];
        tempMesheInfo.boneAssociated = mg.boneAssociated[i];
		tempMesheInfo.Hash = mg.meshHash[i];
		tempMesheInfo.vertCount = mg.verticesCount[i];
		tempMesheInfo.indCount = mg.indicesCount[i];
        tempMesheInfo.buffC = mg.buffCount[i];
        tempMesheInfo.compC = mg.CompCount[i];
        tempMesheInfo.vertOffset = mg.verticesOffsetter[i];
        tempMesheInfo.indOffset = mg.indicesOffsetter[i];
        uint16_t stride = 0;

        for (int b = 0; b < mg.CompCount[i]; b++)
        {
            if (mg.Components[i][b].index == 0)
            {
                stride = Switcher(mg.Components[i], stride, b);
            }
        }
        tempMesheInfo.Stride = stride;

        tempMesheInfo.txcoordCount = 0;

        std::vector<uint16_t> uvdetect_count;
        for (int udc = 0; udc < tempMesheInfo.compC; udc++)
        {
            if ((uint16_t)mg.Components[i][udc].compID >= 3 && (uint16_t)mg.Components[i][udc].compID < 9)
            {
                uvdetect_count.push_back(udc);
                tempMesheInfo.txcoordDetectCounter.push_back(udc);
                tempMesheInfo.txcoordCount++;
            }
        }

        for (int uvf = 0; uvf < tempMesheInfo.txcoordCount; uvf++)
        {
            tempMesheInfo.txcoordSize.push_back(SizeFinder(uvdetect_count[uvf], mg.Components[i]));
        }

        for (int uvo = 0; uvo < tempMesheInfo.txcoordCount; uvo++)
        {
            tempMesheInfo.txcoordOffsets.push_back(OffsetFinder(file, mg.Components[i], tempMesheInfo.buffC, uvdetect_count[uvo], mg.vertexBlockOffsetter[i], tempMesheInfo.vertOffset, tempMesheInfo.compC));
        }

        uint16_t nordetect_count = 0;
        for (int ndc = 0; ndc < tempMesheInfo.compC; ndc++)
        {
            if ((uint16_t)mg.Components[i][ndc].compID == 1)
            {
                nordetect_count = ndc;
                break;
            }
        }

        tempMesheInfo.norDetectCounter = nordetect_count;
        tempMesheInfo.norOffset = OffsetFinder(file, mg.Components[i], tempMesheInfo.buffC, nordetect_count, mg.vertexBlockOffsetter[i], tempMesheInfo.vertOffset, tempMesheInfo.compC);

        uint16_t tandetect_count = 0;
        for (int cdc = 0; cdc < tempMesheInfo.compC; cdc++)
        {
            if ((uint16_t)mg.Components[i][cdc].compID == 2)
            {
                tandetect_count = cdc;
                break;
            }
        }

        tempMesheInfo.tanDetectCounter = tandetect_count;
        tempMesheInfo.tanOffset = OffsetFinder(file, mg.Components[i], tempMesheInfo.buffC, tandetect_count, mg.vertexBlockOffsetter[i], tempMesheInfo.vertOffset, tempMesheInfo.compC);

        uint16_t weigdetect_count = 0;
        for (int wdc = 0; wdc < tempMesheInfo.compC; wdc++)
        {
            if ((uint16_t)mg.Components[i][wdc].compID == 10)
            {
                weigdetect_count = wdc;
                break;
            }
        }

        tempMesheInfo.weightDetectCounter = weigdetect_count;
        tempMesheInfo.weightOffset = OffsetFinder(file, mg.Components[i], tempMesheInfo.buffC, weigdetect_count, mg.vertexBlockOffsetter[i], tempMesheInfo.vertOffset, tempMesheInfo.compC);

        uint16_t bondetect_count = 0;
        for (int bdc = 0; bdc < tempMesheInfo.compC; bdc++)
        {
            if ((uint16_t)mg.Components[i][bdc].compID == 9)
            {
                bondetect_count = bdc;
                break;
            }
        }

        tempMesheInfo.jointDetectCounter = bondetect_count;
        tempMesheInfo.jointOffset  = OffsetFinder(file, mg.Components[i], tempMesheInfo.buffC, bondetect_count, mg.vertexBlockOffsetter[i], tempMesheInfo.vertOffset, tempMesheInfo.compC);

        tempMesheInfo.jointSize = 0;
        for (int bsf = 0; bsf < bondetect_count; bsf++)
        {
            tempMesheInfo.jointSize = SizeFinder(bondetect_count, mg.Components[i]);
        }

        tempMesheInfo.vertSize = mg.Components[i][0].dataType;


        tempMesheInfo.meshScale = mg.meshScale[i];
        tempMesheInfo.meshMin = mg.meshMin[i];

        meshesInfo.push_back(tempMesheInfo);
	}
	return meshesInfo;
}
RawMesh containRawMesh(MeshInfo meshinfo, std::ifstream& file , uint32_t off)
{
    std::cout << "Vertex Offset - " << (meshinfo.vertOffset + off) << " FVF - " << meshinfo.Stride << std::endl;

    RawMesh Mesh(meshinfo.vertCount, meshinfo.indCount);

    for (uint32_t v = 0; v < meshinfo.vertCount; v++)
    {
        file.seekg((uint64_t)meshinfo.vertOffset + (uint64_t)meshinfo.Stride * v + off);
        if (meshinfo.vertSize == 6)
        {
            uint16_t x, y, z;
            file.read((char*)&x, sizeof(uint16_t));
            file.read((char*)&y, sizeof(uint16_t));
            file.read((char*)&z, sizeof(uint16_t));
            Vec3 vec;
            vec.X = (float)x / (float)65535;
            vec.Y = (float)y / (float)65535;
            vec.Z = (float)z / (float)65535;
            Mesh.vertices[v].X = vec.X * meshinfo.meshScale.X + meshinfo.meshMin.X;
            Mesh.vertices[v].Y = vec.Y * meshinfo.meshScale.Y + meshinfo.meshMin.Y;
            Mesh.vertices[v].Z = vec.Z * meshinfo.meshScale.Z + meshinfo.meshMin.Z;
        }
        else
        {
            file.read((char*)&Mesh.vertices[v].X, sizeof(float));
            file.read((char*)&Mesh.vertices[v].Y, sizeof(float));
            file.read((char*)&Mesh.vertices[v].Z, sizeof(float));
        }
    }

    file.seekg((uint64_t)meshinfo.indOffset + (uint64_t)off);
    for (uint32_t i = 0; i < meshinfo.indCount; i++)
    {
        file.read((char*)&Mesh.indices[i], sizeof(uint16_t));
    }

    
    // doing Uv's
    uint32_t UvStride = 0;

    for (int uvno = 0; uvno < meshinfo.txcoordCount; uvno++)
    {
        UvStride = 0;

        if (meshinfo.txcoordDetectCounter[uvno] <= (meshinfo.compC - meshinfo.buffC))
        {
            UvStride = meshinfo.Stride;
        }
        else
        {
            UvStride = meshinfo.txcoordSize[uvno];
        }
        for (uint32_t i = 0; i < meshinfo.vertCount; i++)
        {
            Vec2 vec;
            file.seekg((uint64_t)meshinfo.txcoordOffsets[uvno] + (uint64_t)UvStride * i + off);
            if (meshinfo.txcoordSize[uvno] / 2 == 2)
            {
                uint16_t x, y;
                file.read((char*)&x, sizeof(uint16_t));
                file.read((char*)&y, sizeof(uint16_t));
                vec.X = (float)x / (float)65535;
                vec.Y = (float)y / (float)65535;
            }
            else
            {
                file.read((char*)&vec.X, sizeof(float));
                file.read((char*)&vec.Y, sizeof(float));
            }

            if (uvno == 0)
            {
                Mesh.txcoord0[i].X = vec.X;
                Mesh.txcoord0[i].Y = vec.Y;
            }
            else if (uvno == 1)
            {
                Mesh.txcoord1[i].X = vec.X;
                Mesh.txcoord1[i].Y = vec.Y;
            }
            else if (uvno == 2)
            {
                Mesh.txcoord2[i].X = vec.X;
                Mesh.txcoord2[i].Y = vec.Y;
            }
            else
            {
                Mesh.txcoord3[i].X = vec.X;
                Mesh.txcoord3[i].Y = vec.Y;
            }
        }
    }
    // done uv's
    

    // doing normals
    uint32_t NorRead32;

    uint32_t NorStride = 0;
    if (meshinfo.norDetectCounter <= (meshinfo.compC - meshinfo.buffC))
    {
        NorStride = meshinfo.Stride;
    }
    else
    {
        NorStride = 4;
    }
    for (uint32_t i = 0; i < meshinfo.vertCount; i++)
    {
        file.seekg((uint64_t)meshinfo.norOffset + (uint64_t)NorStride * i + off);
        file.read((char*)&NorRead32, sizeof(uint32_t));
        Vec4 vec = TenBitShifted(NorRead32);
        vec.normalize();
        Mesh.normals[i].X = vec.X;
        Mesh.normals[i].Y = vec.Y;
        Mesh.normals[i].Z = vec.Z;
    }
    // done normals
    
    // doing tangents
    uint32_t TanRead32;

    uint32_t TanStride = 0;
    if (meshinfo.tanDetectCounter <= (meshinfo.compC - meshinfo.buffC))
    {
        TanStride = meshinfo.Stride;
    }
    else
    {
        TanStride = 4;
    }
    for (uint32_t i = 0; i < meshinfo.vertCount; i++)
    {
        file.seekg((uint64_t)meshinfo.tanOffset + (uint64_t)TanStride * i + off);

        file.read((char*)&TanRead32, sizeof(uint32_t));
        Vec4 vec = TenBitShifted(TanRead32);
        vec.normalize();
        Mesh.tangents[i].X = vec.X;
        Mesh.tangents[i].Y = vec.Y;
        Mesh.tangents[i].Z = vec.Z;
        Mesh.tangents[i].W = (float)1.0;
    }
    // done tangents

    // doing bones index
    uint32_t BoneStride = 0;
    if (meshinfo.jointDetectCounter <= (meshinfo.compC - meshinfo.buffC))
    {
        BoneStride = meshinfo.Stride;
    }
    else
    {
        BoneStride = meshinfo.jointSize;
    }
    for (uint32_t i = 0; i < meshinfo.vertCount; i++)
    {
        file.seekg((uint64_t)meshinfo.jointOffset + (uint64_t)BoneStride * i + off);

        if (meshinfo.vertOffset != meshinfo.jointOffset)
        {
            if (meshinfo.jointSize / 4 == 2)
            {
                uint16_t x, y, z, w;
                file.read((char*)&x, sizeof(uint16_t));
                file.read((char*)&y, sizeof(uint16_t));
                file.read((char*)&z, sizeof(uint16_t));
                file.read((char*)&w, sizeof(uint16_t));
                Mesh.joints[i][0] = x;
                Mesh.joints[i][1] = y;
                Mesh.joints[i][2] = z;
                Mesh.joints[i][3] = w;
            }
            else
            {
                uint8_t x, y, z, w;
                file.read((char*)&x, sizeof(uint8_t));
                file.read((char*)&y, sizeof(uint8_t));
                file.read((char*)&z, sizeof(uint8_t));
                file.read((char*)&w, sizeof(uint8_t));
                Mesh.joints[i][0] = (uint16_t)x;
                Mesh.joints[i][1] = (uint16_t)y;
                Mesh.joints[i][2] = (uint16_t)z;
                Mesh.joints[i][3] = (uint16_t)w;
            }
        }
        else
        {
            Mesh.joints[i][0] = meshinfo.boneAssociated;
            Mesh.joints[i][1] = 0;
            Mesh.joints[i][2] = 0;
            Mesh.joints[i][3] = 0;
        }
    }
    // done bone index
    // doing weights
    uint32_t wgtRead = 0;
    uint32_t wgtStride = 0;
    if (meshinfo.weightDetectCounter <= (meshinfo.compC - meshinfo.buffC))
    {
        wgtStride = meshinfo.Stride;
    }
    else
    {
        wgtStride = 4;
    }
    for (uint32_t i = 0; i < meshinfo.vertCount; i++)
    {
        if (meshinfo.vertOffset != meshinfo.weightOffset)
        {
            file.seekg((uint64_t)meshinfo.weightOffset + (uint64_t)wgtStride * i + off);
            file.read((char*)&wgtRead, sizeof(uint32_t));

            Vec4 vec = TenBitUnsigned(wgtRead);

            float sum = vec.X + vec.Y + vec.Z;
            float NorRatio = 1 / sum;
            Mesh.weights[i][0] = vec.X * NorRatio;
            Mesh.weights[i][1] = vec.Y * NorRatio;
            Mesh.weights[i][2] = vec.Z * NorRatio;
            Mesh.weights[i][3] = 0.f;
            //Mesh.weights[i][3] = vec.W * 3 /1023.f;
        }
        else
        {
            Mesh.weights[i][0] = 1;
            Mesh.weights[i][1] = 0;
            Mesh.weights[i][2] = 0;
            Mesh.weights[i][3] = 0;
        }
    }
    // done weights
    return Mesh;
}
inline void RawToObj(std::vector<RawMesh> expMeshes)
{
    int startIndex = 0, mIndex = -1;
    std::ofstream file("test.obj");

    for (uint32_t e = 0; e < expMeshes.size(); e++)
    {
        file << "o mesh_" << e << std::endl;

        for (uint32_t i = 0; i < expMeshes[e].VertCount; i++)
        {
            file << "v " << expMeshes[e].vertices[i].X << " " <<expMeshes[e].vertices[i].Y << " " << expMeshes[e].vertices[i].Z << std::endl;
        }
        for (uint32_t i = 0; i < expMeshes[e].VertCount; i++)
        {
            file << "vt  " << expMeshes[e].txcoord0[i].X << " " << expMeshes[e].txcoord0[i].Y << std::endl;
        }
        for (uint32_t i = 0; i < expMeshes[e].VertCount; i++)
        {
            file << "vn  " << expMeshes[e].normals[i].X << " " << expMeshes[e].normals[i].Y << " " << expMeshes[e].normals[i].Z << std::endl;
        }
        // faces
        for (uint32_t i = 0; i < expMeshes[e].IndCount; i += 3)
        {
            file << "f " << expMeshes[e].indices[i + 1] + 1 + startIndex << "/" << expMeshes[e].indices[i + 1] + 1 + startIndex << "/" << expMeshes[e].indices[i + 1] + 1 + startIndex << " ";
            file << expMeshes[e].indices[i] + 1 + startIndex << "/" << expMeshes[e].indices[i] + 1 + startIndex << "/" << expMeshes[e].indices[i] + 1 + startIndex << " ";
            file << expMeshes[e].indices[i + 2] + 1 + startIndex << "/" << expMeshes[e].indices[i + 2] + 1 + startIndex << "/" << expMeshes[e].indices[i + 2] + 1 + startIndex << std::endl;
        }

        startIndex += expMeshes[e].VertCount;
    }
}