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
void WriteRawMeshToStream(MeshInfo& meshInfo, const RawMeshContainer& rawMesh, std::iostream& outStream, const size_t& streamWriteOff)
{
    meshInfo.vertCount = rawMesh.VertCount;
    meshInfo.indCount = rawMesh.IndCount;
    outStream.seekp(streamWriteOff);
    for (size_t idx = 0; idx < meshInfo.bufferOffset.size(); idx++)
    {
        if (idx == 0)
            meshInfo.bufferOffset[idx] = streamWriteOff;
        else
        {
            meshInfo.bufferOffset[idx] = meshInfo.bufferOffset[idx - 1] + ((meshInfo.vertCount * meshInfo.bufferStride[idx - 1] + 15) & ~(15));
        }
        // Allocation
        byte b = 0;
        for (size_t cnt = 0; cnt < ((meshInfo.vertCount * meshInfo.bufferStride[idx] + 15) & ~(15)); cnt++)
            outStream.write((char*)&b, sizeof(b));
    }
    meshInfo.vertexOffset = meshInfo.bufferOffset[0];
    meshInfo.indicesOffset = meshInfo.bufferOffset[meshInfo.bufferOffset.size() - 1] + ((meshInfo.vertCount * meshInfo.bufferStride[meshInfo.bufferOffset.size() - 1] + 15) & ~(15));


    Vec3 Max = Vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    Vec3 Min = Vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());

    for (uint32_t t = 0; t < rawMesh.VertCount; t++)
    {
        Max.X = std::max<float>(rawMesh.vertices[t].X, Max.X);
        Max.Y = std::max<float>(rawMesh.vertices[t].Y, Max.Y);
        Max.Z = std::max<float>(rawMesh.vertices[t].Z, Max.Z);

        Min.X = std::min<float>(rawMesh.vertices[t].X, Min.X);
        Min.Y = std::min<float>(rawMesh.vertices[t].Y, Min.Y);
        Min.Z = std::min<float>(rawMesh.vertices[t].Z, Min.Z);
    }

    meshInfo.meshScale = Vec3(Max.X - Min.X, Max.Y - Min.Y, Max.Z - Min.Z);
    meshInfo.meshMin = Min;

    for (uint32_t t = 0; t < meshInfo.Components.size(); t++)
    {
        switch (meshInfo.Components[t].primitiveType)
        {
        case PrimitiveTypes::POSITION:
            if (rawMesh.vertices != nullptr)
            {
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);
                    if (meshInfo.Components[t].dataType == DataTypes::UNSIGNED_SHORT)
                    {
                        Vec3 vec = rawMesh.vertices[v];
                        vec.X -= meshInfo.meshMin.X;
                        vec.Y -= meshInfo.meshMin.Y;
                        vec.Z -= meshInfo.meshMin.Z;
                        vec.X /= meshInfo.meshScale.X * 65535.f;
                        vec.Y /= meshInfo.meshScale.Y * 65535.f;
                        vec.Z /= meshInfo.meshScale.Z * 65535.f;

                        uint16_t x = uint16_t(vec.X), y = uint16_t(vec.Y), z = uint16_t(vec.Z);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                        outStream.write((char*)&z, sizeof(z));
                    }
                    else
                    {
                        outStream.write((char*)&rawMesh.vertices[v].X, sizeof(float));
                        outStream.write((char*)&rawMesh.vertices[v].Y, sizeof(float));
                        outStream.write((char*)&rawMesh.vertices[v].Z, sizeof(float));
                    }
                }
            }
            break;
        case PrimitiveTypes::NORMALS:
            if (rawMesh.normals != nullptr)
            {
                uint32_t NorWrite32;
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);
                    // Worry about normalization and magnitude when its 0;
                    rawMesh.normals[v].normalize();
                    auto vec = Vec4(rawMesh.normals[v].X, rawMesh.normals[v].Y, rawMesh.normals[v].Z, 1.f);
                    NorWrite32 = UnTenBitShifted(vec);
                    outStream.write((char*)&NorWrite32, sizeof(NorWrite32));
                }
            }
            break;
        case PrimitiveTypes::TANGENTS:
            if (rawMesh.tangents != nullptr)
            {
                uint32_t TanWrite32;
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.Components[t].offset + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v);
                    // Worry about normalization and magnitude when its 0;
                    rawMesh.tangents[v].normalize();
                    auto vec = Vec4(rawMesh.tangents[v].X, rawMesh.tangents[v].Y, rawMesh.tangents[v].Z, 1.f);
                    TanWrite32 = UnTenBitShifted(vec);
                    outStream.write((char*)&TanWrite32, sizeof(TanWrite32));
                }
            }
            break;
        case PrimitiveTypes::TEXCOORD_0:
            if (rawMesh.txcoord0 != nullptr)
            {
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);

                    Vec2 vec = rawMesh.txcoord0[v];

                    if (meshInfo.Components[t].dataType == DataTypes::UNSIGNED_SHORT)
                    {
                        uint16_t x = uint16_t(vec.X * 65535.f), y = uint16_t(vec.Y * 65535.f);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                    }
                    else if (meshInfo.Components[t].dataType == DataTypes::HALFWORD_STRUCT_2)
                    {
                        uint16_t x = uint16_t(vec.X * 32767.f), y = uint16_t(vec.Y * 32767.f);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                    }
                    else
                    {
                        outStream.write((char*)&vec.X, sizeof(float));
                        outStream.write((char*)&vec.Y, sizeof(float));
                    }
                }
            }

            break;
        case PrimitiveTypes::TEXCOORD_1:
            if (rawMesh.txcoord1 != nullptr)
            {
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);

                    Vec2 vec = rawMesh.txcoord1[v];

                    if (meshInfo.Components[t].dataType == DataTypes::UNSIGNED_SHORT)
                    {
                        uint16_t x = uint16_t(vec.X * 65535.f), y = uint16_t(vec.Y * 65535.f);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                    }
                    else if (meshInfo.Components[t].dataType == DataTypes::HALFWORD_STRUCT_2)
                    {
                        uint16_t x = uint16_t(vec.X * 32767.f), y = uint16_t(vec.Y * 32767.f);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                    }
                    else
                    {
                        outStream.write((char*)&vec.X, sizeof(float));
                        outStream.write((char*)&vec.Y, sizeof(float));
                    }
                }
            }
            break;
        case PrimitiveTypes::TEXCOORD_2:
            if (rawMesh.txcoord2 != nullptr)
            {
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);

                    Vec2 vec = rawMesh.txcoord2[v];

                    if (meshInfo.Components[t].dataType == DataTypes::UNSIGNED_SHORT)
                    {
                        uint16_t x = uint16_t(vec.X * 65535.f), y = uint16_t(vec.Y * 65535.f);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                    }
                    else if (meshInfo.Components[t].dataType == DataTypes::HALFWORD_STRUCT_2)
                    {
                        uint16_t x = uint16_t(vec.X * 32767.f), y = uint16_t(vec.Y * 32767.f);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                    }
                    else
                    {
                        outStream.write((char*)&vec.X, sizeof(float));
                        outStream.write((char*)&vec.Y, sizeof(float));
                    }
                }
            }
            break;
        case PrimitiveTypes::JOINTS0:
            if (rawMesh.joints != nullptr)
            {
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);

                    if (meshInfo.Components[t].dataType == DataTypes::BYTE_STRUCT_0)
                    {
                        uint8_t x = uint8_t(rawMesh.joints[v][0]), y = uint8_t(rawMesh.joints[v][1]), z = uint8_t(rawMesh.joints[v][2]), w = uint8_t(rawMesh.joints[v][3]);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                        outStream.write((char*)&z, sizeof(z));
                        outStream.write((char*)&w, sizeof(w));
                    }
                    else
                    {
                        uint16_t x = uint16_t(rawMesh.joints[v][0]), y = uint16_t(rawMesh.joints[v][1]), z = uint16_t(rawMesh.joints[v][2]), w = uint16_t(rawMesh.joints[v][3]);
                        outStream.write((char*)&x, sizeof(x));
                        outStream.write((char*)&y, sizeof(y));
                        outStream.write((char*)&z, sizeof(z));
                        outStream.write((char*)&w, sizeof(w));
                    }
                }
            }
            break;
        case PrimitiveTypes::WEIGHTS0:
            if (rawMesh.weights != nullptr)
            {
                for (size_t v = 0; v < meshInfo.vertCount; v++)
                {
                    outStream.seekp(meshInfo.bufferOffset[meshInfo.Components[t].bufferIndex] + meshInfo.bufferStride[meshInfo.Components[t].bufferIndex] * v + meshInfo.Components[t].offset);

                    Vec4 vec = Vec4(rawMesh.weights[v][0], rawMesh.weights[v][1], rawMesh.weights[v][2], 0.f);
                    float sum = vec.X + vec.Y + vec.Z;
                    float NorRatio = 1 / sum;
                    vec.X *= NorRatio;
                    vec.Y *= NorRatio;
                    vec.Z *= NorRatio;

                    uint32_t wgtWrite = UnTenBitUnsigned(vec);
                    // Experiment, Check whats in the 2 MSB bits (any weight info, any?)

                    outStream.write((char*)&wgtWrite, sizeof(wgtWrite));
                }
            }
            break;
        default:
            break;
        }
    }

    outStream.seekp(meshInfo.indicesOffset);
    for (size_t v = 0; v < meshInfo.indCount; v++)
    {
        outStream.write((char*)&rawMesh.indices[v], sizeof(uint16_t));
    }
    size_t indSize = meshInfo.indCount * sizeof(uint16_t);
    size_t padSize = ((indSize + 15) & (~15)) - indSize;
    if (padSize > 0)
    {
        byte* padBytes = new byte[padSize];
        std::fill(padBytes, padBytes + padSize, 0);
        outStream.write((char*)padBytes, padSize);
        delete[] padBytes;
    }
}