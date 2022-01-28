#include <pch.h>
#include "MainFunctions.h"
#include "MathFunctions.h"
#include "../inc/Mesh.h"
#include "../inc/Formats.h"

RawMeshContainer containRawMesh(MeshInfo& meshinfo, std::stringstream& file,std::string name, uint64_t off)
{
    RawMeshContainer Mesh;
    Mesh.name = name;
    Mesh.VertCount = meshinfo.vertCount;
    Mesh.IndCount = meshinfo.indCount;

    Mesh.joints = new uint16_t * [Mesh.VertCount];
    for (uint32_t i = 0; i < Mesh.VertCount; i++)
        Mesh.joints[i] = new uint16_t[4];
    Mesh.weights = new float* [Mesh.VertCount];
    for (uint32_t i = 0; i < Mesh.VertCount; i++)
        Mesh.weights[i] = new float[4];

    for (uint32_t v = 0; v < meshinfo.vertCount; v++)
    {
        Mesh.joints[v][0] = meshinfo.boneAssociated;
        Mesh.joints[v][1] = 0;
        Mesh.joints[v][2] = 0;
        Mesh.joints[v][3] = 0;

        Mesh.weights[v][0] = 1;
        Mesh.weights[v][1] = 0;
        Mesh.weights[v][2] = 0;
        Mesh.weights[v][3] = 0;
    }

    for (uint32_t i = 0; i < meshinfo.Components.size(); i++)
    {
        switch (meshinfo.Components[i].primitiveType)
        {
        case PrimitiveTypes::POSITION:
            Mesh.vertices = new Vec3[Mesh.VertCount];
            //cout << "POSITION" << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";
            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);
                if (meshinfo.Components[i].dataType == DataTypes::UNSIGNED_SHORT)
                {
                    uint16_t x, y, z;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    file.read((char*)&z, sizeof(uint16_t));
                    Vec3 vec;
                    vec.X = (float)x / 65535.f;
                    vec.Y = (float)y / 65535.f;
                    vec.Z = (float)z / 65535.f;
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
            break;
        case PrimitiveTypes::NORMALS:
            Mesh.normals = new Vec3[Mesh.VertCount];
            //cout << "NORMAL" << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            uint32_t NorRead32;
            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);
                file.read((char*)&NorRead32, sizeof(uint32_t));
                Vec4 vec = TenBitShifted(NorRead32);
                vec.normalize();
                Mesh.normals[v].X = vec.X;
                Mesh.normals[v].Y = vec.Y;
                Mesh.normals[v].Z = vec.Z;
            }
            break;
        case PrimitiveTypes::TANGENTS:
            Mesh.tangents = new Vec4[Mesh.VertCount];
            //cout << "TANGENT" << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            uint32_t TanRead32;
            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);
                file.read((char*)&TanRead32, sizeof(uint32_t));
                Vec4 vec = TenBitShifted(TanRead32);
                vec.normalize();
                Mesh.tangents[v].X = vec.X;
                Mesh.tangents[v].Y = vec.Y;
                Mesh.tangents[v].Z = vec.Z;
                Mesh.tangents[v].W = 1.f;
            }
            break;
        case PrimitiveTypes::TEXCOORD_0:
            Mesh.txcoord0 = new Vec2[Mesh.VertCount];
            //cout << meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + off <<  " TX0" << " " << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                Vec2 vec;
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);

                if (meshinfo.Components[i].dataType == DataTypes::UNSIGNED_SHORT)
                {
                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    vec.X = (float)x / 65535.f;
                    vec.Y = (float)y / 65535.f;
                }
                else if (meshinfo.Components[i].dataType == DataTypes::HALFWORD_STRUCT_2)
                {
                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    vec.X = (float)x / 32767.f;
                    vec.Y = (float)y / 32767.f;
                }
                else
                {
                    file.read((char*)&vec.X, sizeof(float));
                    file.read((char*)&vec.Y, sizeof(float));
                }

                Mesh.txcoord0[v].X = vec.X;
                Mesh.txcoord0[v].Y = vec.Y;
            }
            break;
        case PrimitiveTypes::TEXCOORD_1:
            Mesh.txcoord1 = new Vec2[Mesh.VertCount];
            //cout << "TX1" << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                Vec2 vec;
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);

                if (meshinfo.Components[i].dataType == DataTypes::UNSIGNED_SHORT)
                {
                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    vec.X = (float)x / 65535.f;
                    vec.Y = (float)y / 65535.f;
                }
                else if (meshinfo.Components[i].dataType == DataTypes::HALFWORD_STRUCT_2)
                {
                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    vec.X = (float)x / 32767.f;
                    vec.Y = (float)y / 32767.f;
                }
                else
                {
                    file.read((char*)&vec.X, sizeof(float));
                    file.read((char*)&vec.Y, sizeof(float));
                }

                Mesh.txcoord1[v].X = vec.X;
                Mesh.txcoord1[v].Y = vec.Y;
            }
            break;
        case PrimitiveTypes::TEXCOORD_2:
            Mesh.txcoord2 = new Vec2[Mesh.VertCount];
            //cout << "TX2" << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                Vec2 vec;
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);

                if (meshinfo.Components[i].dataType == DataTypes::UNSIGNED_SHORT)
                {
                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    vec.X = (float)x / 65535.f;
                    vec.Y = (float)y / 65535.f;
                }
                else if (meshinfo.Components[i].dataType == DataTypes::HALFWORD_STRUCT_2)
                {
                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    vec.X = (float)x / 32767.f;
                    vec.Y = (float)y / 32767.f;
                }
                else
                {
                    file.read((char*)&vec.X, sizeof(float));
                    file.read((char*)&vec.Y, sizeof(float));
                }

                Mesh.txcoord2[v].X = vec.X;
                Mesh.txcoord2[v].Y = vec.Y;
            }
            break;
        case PrimitiveTypes::JOINTS0:
            //cout << "Joint0" << meshinfo.vertexOffset << " " << (uint16_t)meshinfo.Components[i].offset << " " << (uint16_t)meshinfo.Components[i].bufferIndex << " " << (uint16_t)meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << (uint16_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);

                if (meshinfo.Components[i].dataType == DataTypes::BYTE_STRUCT_0)
                {
                    uint8_t x, y, z, w;
                    file.read((char*)&x, sizeof(uint8_t));
                    file.read((char*)&y, sizeof(uint8_t));
                    file.read((char*)&z, sizeof(uint8_t));
                    file.read((char*)&w, sizeof(uint8_t));
                    Mesh.joints[v][0] = x;
                    Mesh.joints[v][1] = y;
                    Mesh.joints[v][2] = z;
                    Mesh.joints[v][3] = w;
                }
                else
                {
                    uint16_t x, y, z, w;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    file.read((char*)&z, sizeof(uint16_t));
                    file.read((char*)&w, sizeof(uint16_t));
                    Mesh.joints[v][0] = x;
                    Mesh.joints[v][1] = y;
                    Mesh.joints[v][2] = z;
                    Mesh.joints[v][3] = w;
                }
            }
            break;
        case PrimitiveTypes::WEIGHTS0:
            //cout << "Wgt0" << meshinfo.vertexOffset << " " << meshinfo.Components[i].offset << " " << meshinfo.Components[i].bufferIndex << " " << meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] << " " << meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] << "\n";

            uint32_t wgtRead;
            for (uint32_t v = 0; v < meshinfo.vertCount; v++)
            {
                file.seekg(meshinfo.Components[i].offset + meshinfo.bufferOffset[meshinfo.Components[i].bufferIndex] + (uint64_t)meshinfo.bufferStride[meshinfo.Components[i].bufferIndex] * (uint64_t)v + off);

                file.read((char*)&wgtRead, sizeof(uint32_t));

                Vec4 vec = TenBitUnsigned(wgtRead);

                float sum = vec.X + vec.Y + vec.Z;
                float NorRatio = 1 / sum;
                Mesh.weights[v][0] = vec.X * NorRatio;
                Mesh.weights[v][1] = vec.Y * NorRatio;
                Mesh.weights[v][2] = vec.Z * NorRatio;
                Mesh.weights[v][3] = 0.f;
            }
            break;
        default:
            break;
        }
    }

    Mesh.indices = new uint16_t[Mesh.IndCount];
    file.seekg((uint64_t)meshinfo.indicesOffset + (uint64_t)off);
    for (uint32_t i = 0; i < meshinfo.indCount; i++)
    {
        file.read((char*)&Mesh.indices[i], sizeof(uint16_t));
    }

    return Mesh;
}