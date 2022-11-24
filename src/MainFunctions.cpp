#include <pch.h>
#include "MainFunctions.h"
#include "MathFunctions.h"
#include "../inc/Mesh.h"
#include "../inc/Formats.h"

RawMeshContainer containRawMesh(MeshInfo& meshinfo, std::iostream& file,std::string name, uint64_t off)
{
    file.seekg(0, ios::end);
    size_t size = file.tellg();
    std::unique_ptr<char[]> arr = std::make_unique<char[]>(size);
    file.seekg(0, std::ios::beg);
    file.read(arr.get(), size);

    RawMeshContainer Mesh;
    Mesh.Name = name;
    Mesh.VertCount = meshinfo.vertCount;
    Mesh.IndCount = meshinfo.indCount;

    Mesh.Joints = vector<vector<uint16_t>>(Mesh.VertCount, vector<uint16_t>(16));
    Mesh.Weights = vector<vector<float>>(Mesh.VertCount, vector<float>(16));

    for (uint32_t v = 0; v < meshinfo.vertCount; v++)
    {
        Mesh.Joints[v][0] = meshinfo.parentBone;
        Mesh.Weights[v][0] = 1.f;
    }

    for (auto &comp : meshinfo.Components)
    {
        switch (comp.primitiveType)
        {
        case PrimitiveTypes::POSITION:
            Mesh.Positions = vector<Vec3>(Mesh.VertCount);
            if (comp.dataType == DataTypes::R16_UNORM)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);
                    
                    uint16_t x, y, z;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    file.read((char*)&z, sizeof(uint16_t));
                    Vec3 vec;
                    vec.X = (float)x / 65535.f;
                    vec.Y = (float)y / 65535.f;
                    vec.Z = (float)z / 65535.f;
                    Mesh.Positions[v].X = vec.X * meshinfo.meshScale.X + meshinfo.meshMin.X;
                    Mesh.Positions[v].Y = vec.Y * meshinfo.meshScale.Y + meshinfo.meshMin.Y;
                    Mesh.Positions[v].Z = vec.Z * meshinfo.meshScale.Z + meshinfo.meshMin.Z;
                }
            }
            else if (comp.dataType == DataTypes::R32_FLOAT)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);
                    
                    file.read((char*)&Mesh.Positions[v].X, sizeof(float));
                    file.read((char*)&Mesh.Positions[v].Y, sizeof(float));
                    file.read((char*)&Mesh.Positions[v].Z, sizeof(float));
                }
            }
            else throw std::exception("Position Invalid Data Type: " + int(comp.dataType));

            break;
        case PrimitiveTypes::NORMALS:
            Mesh.Normals = vector<Vec3>(Mesh.VertCount);

            if (comp.dataType == DataTypes::R10G10B10A2_TYPELESS)
            {
                uint32_t R10G10B10A2;
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);
                    
                    file.read((char*)&R10G10B10A2, sizeof(uint32_t));
                    Vec4 vec = R10G10B10A2_SNORM_TO_VEC4(R10G10B10A2);
                    vec.normalize();
                    Mesh.Normals[v] = Vec3(vec.X, vec.Y, vec.Z);
                }
            }
            else throw std::exception("Normals Invalid Data Type: " + int(comp.dataType));

            break;
        case PrimitiveTypes::TANGENTS:
            Mesh.Tangents = vector<Vec4>(Mesh.VertCount);

            if (comp.dataType == DataTypes::R10G10B10A2_TYPELESS)
            {
                uint32_t R10G10B10A2;
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);
                    
                    file.read((char*)&R10G10B10A2, sizeof(uint32_t));
                    Vec4 vec = R10G10B10A2_SNORM_TO_VEC4(R10G10B10A2);

                    vec.normalize();
                    Mesh.Tangents[v] = Vec4(vec.X, vec.Y, vec.Z, 1.f);
                }
            }
            else throw std::exception("Tangent Invalid Data Type: " + int(comp.dataType));

            break;
        case PrimitiveTypes::TEXCOORD_0:
        case PrimitiveTypes::TEXCOORD_1:
        case PrimitiveTypes::TEXCOORD_2:
        case PrimitiveTypes::TEXCOORD_3:
        {
            auto texcoord = vector<Vec2>(Mesh.VertCount);

            if (comp.dataType == DataTypes::R16_UNORM)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);

                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));

                    texcoord[v] = Vec2(x / 65535.f, y / 65535.f);
                }
            }
            else if (comp.dataType == DataTypes::R16_SNORM)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);

                    uint16_t x, y;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));

                    texcoord[v] = Vec2((x - 32767.f) / 32768.f, (y - 32767.f )/ 32768.f);
                }
            }
            else if (comp.dataType == DataTypes::R32_FLOAT)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);
                    
                    file.read((char*)&texcoord[v].X, sizeof(float));
                    file.read((char*)&texcoord[v].Y, sizeof(float));
                }
            }
            else throw std::exception("Texcoord Invalid Data Type: " + int(comp.dataType));

            switch (comp.primitiveType)
            {
            case PrimitiveTypes::TEXCOORD_0:
                Mesh.TexCoord0 = texcoord;
                break;
            case PrimitiveTypes::TEXCOORD_1:
                Mesh.TexCoord1 = texcoord;
                break;
            case PrimitiveTypes::TEXCOORD_2:
                Mesh.TexCoord2 = texcoord;
                break;
            case PrimitiveTypes::TEXCOORD_3:
                Mesh.TexCoord3 = texcoord;
                break;
            default:
                break;
            }
        }
            break;
        case PrimitiveTypes::JOINTS0:
            if (comp.dataType == DataTypes::R8_UINT)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);
                    
                    uint8_t x, y, z, w;
                    file.read((char*)&x, sizeof(uint8_t));
                    file.read((char*)&y, sizeof(uint8_t));
                    file.read((char*)&z, sizeof(uint8_t));
                    file.read((char*)&w, sizeof(uint8_t));
                    Mesh.Joints[v][0] = x;
                    Mesh.Joints[v][1] = y;
                    Mesh.Joints[v][2] = z;
                    Mesh.Joints[v][3] = w;
                }
            }
            else if (comp.dataType == DataTypes::R16_UINT)
            {
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);

                    uint16_t x, y, z, w;
                    file.read((char*)&x, sizeof(uint16_t));
                    file.read((char*)&y, sizeof(uint16_t));
                    file.read((char*)&z, sizeof(uint16_t));
                    file.read((char*)&w, sizeof(uint16_t));
                    Mesh.Joints[v][0] = x;
                    Mesh.Joints[v][1] = y;
                    Mesh.Joints[v][2] = z;
                    Mesh.Joints[v][3] = w;
                }
            }
            else if (comp.dataType == DataTypes::R32_UNKNOWN)
            {
                cout << "";

                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);

                    uint32_t residual = 0;
                    uint8_t cnt1 = 1;
                    uint8_t cnt2 = 0;
                    uint32_t mask = 1023;

                    for (uint8_t s = 0; s < comp.elementCount; s++)
                    {
                        uint64_t R1G11B11 = 0;
                        file.read((char*)&R1G11B11, sizeof(uint32_t));

                        uint16_t X = (R1G11B11 & mask);
                        X <<= cnt2;
                        X |= residual;
                        X <<= (cnt1 - cnt2);

                        cnt1++;
                        cnt2 = s;
                        mask >>= 1;

                        uint16_t Y = ((R1G11B11 >> (10 - s)) & 2047);
                        uint16_t Z = ((R1G11B11 >> (21 - s)) & 2047);

                        residual = (R1G11B11 >> (32 - s));

                        Mesh.Joints[v][0 + s * 3] = X;
                        Mesh.Joints[v][1 + s * 3] = Y;
                        Mesh.Joints[v][2 + s * 3] = Z;

                    }
                }
            }
            else throw std::exception("Joint Invalid Data Type: " + int(comp.dataType));

            break;
        case PrimitiveTypes::WEIGHTS0:

            if (comp.dataType == DataTypes::R10G10B10A2_TYPELESS)
            {
                uint32_t R10G10B10A2;
                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);

                    Vec4 vec;
                    float sum = 0;

                    for (uint8_t s = 0; s < comp.elementCount; s++)
                    {
                        file.read((char*)&R10G10B10A2, sizeof(uint32_t));
                        vec = R10G10B10A2_UNORM_TO_VEC4(R10G10B10A2);


                        Mesh.Weights[v][0 + s * 4] = vec.X;
                        Mesh.Weights[v][1 + s * 4] = vec.Y;
                        Mesh.Weights[v][2 + s * 4] = vec.Z;
                        Mesh.Weights[v][3 + s * 4] = vec.W;

                        sum += vec.X + vec.Y + vec.Z + vec.W;
                    }

                    float NorRatio = 1.f / sum;
                    
                    for (uint8_t s = 0; s < comp.elementCount; s++)
                    {
                        Mesh.Weights[v][0 + s * 4] *= NorRatio;
                        Mesh.Weights[v][1 + s * 4] *= NorRatio;
                        Mesh.Weights[v][2 + s * 4] *= NorRatio;
                        Mesh.Weights[v][3 + s * 4] *= NorRatio;
                    }
                }
            }
            else if (comp.dataType == DataTypes::R32_UNKNOWN)
            {
                cout << "";
                uint32_t R10G10B10A2;

                for (uint32_t v = 0; v < meshinfo.vertCount; v++)
                {
                    file.seekg(comp.offset + meshinfo.bufferOffset[comp.bufferIndex] + meshinfo.bufferStride[comp.bufferIndex] * v + off);

                    Vec4 vec;
                    float sum = 0;

                    for (uint8_t s = 0; s < comp.elementCount; s++)
                    {
                        file.read((char*)&R10G10B10A2, sizeof(uint32_t));
                        vec = R10G10B10A2_UNORM_TO_VEC4(R10G10B10A2);


                        Mesh.Weights[v][0 + s * 4] = vec.X;
                        Mesh.Weights[v][1 + s * 4] = vec.Y;
                        Mesh.Weights[v][2 + s * 4] = vec.Z;
                        Mesh.Weights[v][3 + s * 4] = vec.W;

                        sum += vec.X + vec.Y + vec.Z + vec.W;
                    }
                    if (sum < 0.98 || sum > 1.02)
                    {
                        cout << "";
                    }

                    float NorRatio = 1.f / sum;

                    for (uint8_t s = 0; s < comp.elementCount; s++)
                    {
                        Mesh.Weights[v][0 + s * 4] *= NorRatio;
                        Mesh.Weights[v][1 + s * 4] *= NorRatio;
                        Mesh.Weights[v][2 + s * 4] *= NorRatio;
                        Mesh.Weights[v][3 + s * 4] *= NorRatio;
                    }
                }
            }
            else throw std::exception("Weight Invalid Data Type: " + int(comp.dataType));

            break;
        default:
            break;
        }
    }

    Mesh.Indices = vector<uint32_t>(Mesh.IndCount);
    file.seekg(meshinfo.indicesOffset + off);

    if (meshinfo.indicesStride == 2)
    {
        for (uint32_t i = 0; i < meshinfo.indCount; i++)
        {
            uint16_t temp = 0;
            file.read((char*)&temp, sizeof uint16_t );
            Mesh.Indices[i] = temp;
        }
    }
    else if (meshinfo.indicesStride == 4)
    {
        for (uint32_t i = 0; i < meshinfo.indCount; i++)
        {
            file.read((char*)&Mesh.Indices[i], sizeof uint32_t );
        }
    }
    else
    {
        throw std::exception("Invalid Indices Stride");
    }

    return Mesh;
}