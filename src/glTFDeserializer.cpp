#include "pch.h"
#include "glTFDeserializer.h"

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

#include <filesystem>

#include <fstream>
#include <sstream>
#include <iostream>

#include <cassert>
#include <cstdlib>

using namespace Microsoft::glTF;

// The glTF SDK is decoupled from all file I/O by the IStreamReader (and IStreamWriter)
// interface(s) and the C++ stream-based I/O library. This allows the glTF SDK to be used in
// sandboxed environments, such as WebAssembly modules and UWP apps, where any file I/O code
// must be platform or use-case specific.
class StreamReader : public IStreamReader
{
public:
    StreamReader(std::filesystem::path pathBase) : m_pathBase(std::move(pathBase))
    {
        assert(m_pathBase.has_root_path());
    }

    // Resolves the relative URIs of any external resources declared in the glTF manifest
    std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
    {
        // In order to construct a valid stream:
        // 1. The filename argument will be encoded as UTF-8 so use filesystem::path to
        //    correctly construct a path instance.
        // 2. Generate an absolute path by concatenating m_pathBase with the specified filename
        //    path. The filesystem::operator/ uses the platform's preferred directory separator
        //    if appropriate.
        // 3. Always open the file stream in binary mode. The glTF SDK will handle any text
        //    encoding issues for us.
        auto streamPath = m_pathBase / std::filesystem::path(filename);
        auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);

        // Check if the stream has no errors and is ready for I/O operations
        if (!stream || !(*stream))
        {
            throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
        }

        return stream;
    }

private:
    std::filesystem::path m_pathBase;
};

// Uses the Document class to print some basic information about various top-level glTF entities
void PrintDocumentInfo(const Document& document)
{
    // Asset Info
    std::cout << "Asset Version:    " << document.asset.version << "\n";
    std::cout << "Asset MinVersion: " << document.asset.minVersion << "\n";
    std::cout << "Asset Generator:  " << document.asset.generator << "\n";
    std::cout << "Asset Copyright:  " << document.asset.copyright << "\n\n";

    // Scene Info
    std::cout << "Scene Count: " << document.scenes.Size() << "\n";

    if (document.scenes.Size() > 0U)
    {
        std::cout << "Default Scene Index: " << document.GetDefaultScene().id << "\n\n";
    }
    else
    {
        std::cout << "\n";
    }

    // Entity Info
    std::cout << "Node Count:     " << document.nodes.Size() << "\n";
    std::cout << "Camera Count:   " << document.cameras.Size() << "\n";
    std::cout << "Material Count: " << document.materials.Size() << "\n\n";

    // Mesh Info
    std::cout << "Mesh Count: " << document.meshes.Size() << "\n";
    std::cout << "Skin Count: " << document.skins.Size() << "\n\n";

    // Texture Info
    std::cout << "Image Count:   " << document.images.Size() << "\n";
    std::cout << "Texture Count: " << document.textures.Size() << "\n";
    std::cout << "Sampler Count: " << document.samplers.Size() << "\n\n";

    // Buffer Info
    std::cout << "Buffer Count:     " << document.buffers.Size() << "\n";
    std::cout << "BufferView Count: " << document.bufferViews.Size() << "\n";
    std::cout << "Accessor Count:   " << document.accessors.Size() << "\n\n";

    // Animation Info
    std::cout << "Animation Count: " << document.animations.Size() << "\n\n";

    for (const auto& extension : document.extensionsUsed)
    {
        std::cout << "Extension Used: " << extension << "\n";
    }

    if (!document.extensionsUsed.empty())
    {
        std::cout << "\n";
    }

    for (const auto& extension : document.extensionsRequired)
    {
        std::cout << "Extension Required: " << extension << "\n";
    }

    if (!document.extensionsRequired.empty())
    {
        std::cout << "\n";
    }
}

bool GetRawMeshesFromGLTF(const Document& document, const GLTFResourceReader& resourceReader, std::vector<RawMeshContainer>& Meshes, const Rig& Armature)
{
    auto skin = document.skins.Elements().front();
    auto nodesAll =  document.nodes.Elements();

    uint16_t* reTargIdxLookUp = new uint16_t[skin.jointIds.size()];

    for (int i = 0; i < skin.jointIds.size(); i++)
    {
        string boneName = nodesAll[std::stoul(skin.jointIds[i])].name;
        bool found = false;
        for (int j = 0; j < Armature.boneCount; j++)
        {
            if (Armature.boneNames[j] == boneName)
            {
                reTargIdxLookUp[i] = j;
                found = true;
            }
        }
        if (!found)
        {
            throw std::exception(("{ " + boneName + " }" + " Not Present in the Original Rig In .Wad File").c_str());
        }
    }
    // Use the resource reader to get each mesh primitive's position data
    for (const auto& mesh : document.meshes.Elements())
    {
//        if (mesh.name != "submesh_0029_0")
//            continue;
        RawMeshContainer meshContainer;
        meshContainer.name = mesh.name;
        for (const auto& meshPrimitive : mesh.primitives)
        {
            std::string accessorId;
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.VertCount = accessor.count;
                meshContainer.vertices = new Vec3[meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.vertices[i] = Vec3(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_NORMAL, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.normals = new Vec3[meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.normals[i] = Vec3(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_TANGENT, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.tangents = new Vec4[meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.tangents[i] = Vec4(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]);
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_TEXCOORD_0, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.txcoord0 = new Vec2[meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.txcoord0[i] = Vec2(data[i * 2], data[i * 2 + 1]);
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_TEXCOORD_1, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.txcoord1 = new Vec2[meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.txcoord1[i] = Vec2(data[i * 2], data[i * 2 + 1]);
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId("TEXCOORD_2", accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.txcoord2 = new Vec2[meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.txcoord2[i] = Vec2(data[i * 2], data[i * 2 + 1]);
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_JOINTS_0, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                switch (accessor.componentType)
                {
                case COMPONENT_UNSIGNED_BYTE:
                {
                    const auto data = resourceReader.ReadBinaryData<uint8_t>(document, accessor);
                    meshContainer.joints = new uint16_t * [meshContainer.VertCount];
                    for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                        meshContainer.joints[i] = new uint16_t[4];
                    for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                    {
                        meshContainer.joints[i][0] = reTargIdxLookUp[data[i * 4]];
                        meshContainer.joints[i][1] = reTargIdxLookUp[data[i * 4 + 1]];
                        meshContainer.joints[i][2] = reTargIdxLookUp[data[i * 4 + 2]];
                        meshContainer.joints[i][3] = reTargIdxLookUp[data[i * 4 + 3]];
                    }
                }
                break;
                case COMPONENT_UNSIGNED_SHORT:
                {
                    const auto data = resourceReader.ReadBinaryData<uint16_t>(document, accessor);
                    meshContainer.joints = new uint16_t * [meshContainer.VertCount];
                    for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                        meshContainer.joints[i] = new uint16_t[4];
                    for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                    {
                        meshContainer.joints[i][0] = reTargIdxLookUp[data[i * 4]];
                        meshContainer.joints[i][1] = reTargIdxLookUp[data[i * 4 + 1]];
                        meshContainer.joints[i][2] = reTargIdxLookUp[data[i * 4 + 2]];
                        meshContainer.joints[i][3] = reTargIdxLookUp[data[i * 4 + 3]];
                    }
                }
                break;
                default:
                    throw GLTFException("Unsupported accessor ComponentType");
                }
            }
            if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_WEIGHTS_0, accessorId))
            {
                const Accessor& accessor = document.accessors.Get(accessorId);

                const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                meshContainer.weights = new float* [meshContainer.VertCount];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                    meshContainer.weights[i] = new float[4];
                for (uint32_t i = 0; i < meshContainer.VertCount; i++)
                {
                    meshContainer.weights[i][0] = data[i * 4];
                    meshContainer.weights[i][1] = data[i * 4 + 1];
                    meshContainer.weights[i][2] = data[i * 4 + 2];
                    meshContainer.weights[i][3] = data[i * 4 + 3];
                }
            }
            if (meshContainer.indices == nullptr)
            {
                const Accessor& accessor = document.accessors.Get(meshPrimitive.indicesAccessorId);

                const auto data = resourceReader.ReadBinaryData<uint16_t>(document, accessor);

                meshContainer.IndCount = accessor.count;
                meshContainer.indices = new uint32_t[meshContainer.IndCount];

                for (uint32_t i = 0; i < meshContainer.IndCount; i+=3)
                {
                    meshContainer.indices[i] = data[i + 1];
                    meshContainer.indices[i + 1] = data[i];
                    meshContainer.indices[i + 2] = data[i + 2];
                }
            }
        }
        Meshes.push_back(meshContainer);
    }
    return true;
}

bool ReadGLTF(const std::filesystem::path& path, Document& document, std::shared_ptr<GLTFResourceReader>& resourceReader)
{
    // Pass the absolute path, without the filename, to the stream reader
    auto streamReader = std::make_unique<StreamReader>(path.parent_path());

    std::filesystem::path pathFile = path.filename();
    std::filesystem::path pathFileExt = pathFile.extension();

    std::string manifest;

    auto MakePathExt = [](const std::string& ext)
    {
        return "." + ext;
    };

    // If the file has a '.gltf' extension then create a GLTFResourceReader
    if (pathFileExt == MakePathExt(GLTF_EXTENSION))
    {
        auto gltfStream = streamReader->GetInputStream(pathFile.string()); // Pass a UTF-8 encoded filename to GetInputString
        auto gltfResourceReader = std::make_unique<GLTFResourceReader>(std::move(streamReader));
        std::stringstream manifestStream;

        // Read the contents of the glTF file into a string using a std::stringstream
        manifestStream << gltfStream->rdbuf();
        manifest = manifestStream.str();

        resourceReader = std::move(gltfResourceReader);
    }

    // If the file has a '.glb' extension then create a GLBResourceReader. This class derives
    // from GLTFResourceReader and adds support for reading manifests from a GLB container's
    // JSON chunk and resource data from the binary chunk.
    if (pathFileExt == MakePathExt(GLB_EXTENSION))
    {
        auto glbStream = streamReader->GetInputStream(pathFile.string()); // Pass a UTF-8 encoded filename to GetInputString
        auto glbResourceReader = std::make_unique<GLBResourceReader>(std::move(streamReader), std::move(glbStream));

        manifest = glbResourceReader->GetJson(); // Get the manifest from the JSON chunk

        resourceReader = std::move(glbResourceReader);
    }

    if (!resourceReader)
    {
        return false;
    }

    try
    {
        document = Deserialize(manifest);
    }
    catch (const GLTFException& ex)
    {
        return false;
    }
    return true;
}