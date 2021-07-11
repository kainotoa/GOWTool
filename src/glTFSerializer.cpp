#include "glTFSerializer.h"
#include "pch.h"

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Serialize.h>

#include <sstream>
#include <cassert>
#include <cstdlib>

using namespace Microsoft::glTF;

// The glTF SDK is decoupled from all file I/O by the IStreamWriter (and IStreamReader)
// interface(s) and the C++ stream-based I/O library. This allows the glTF SDK to be used in
// sandboxed environments, such as WebAssembly modules and UWP apps, where any file I/O code
// must be platform or use-case specific.
class StreamWriter : public IStreamWriter
{
public:
    StreamWriter(std::filesystem::path pathBase) : m_pathBase(std::move(pathBase))
    {
        assert(m_pathBase.has_root_path());
    }

    // Resolves the relative URIs of any external resources declared in the glTF manifest
    std::shared_ptr<std::ostream> GetOutputStream(const std::string& filename) const override
    {
        // In order to construct a valid stream:
        // 1. The filename argument will be encoded as UTF-8 so use filesystem::u8path to
        //    correctly construct a path instance.
        // 2. Generate an absolute path by concatenating m_pathBase with the specified filename
        //    path. The filesystem::operator/ uses the platform's preferred directory separator
        //    if appropriate.
        // 3. Always open the file stream in binary mode. The glTF SDK will handle any text
        //    encoding issues for us.
        auto streamPath = m_pathBase / std::filesystem::path(filename);
        auto stream = std::make_shared<std::ofstream>(streamPath, std::ios_base::binary);

        // Check if the stream has no errors and is ready for I/O operations
        if (!stream || !(*stream))
        {
            throw std::runtime_error("Unable to create a valid output stream for uri: " + filename);
        }

        return stream;
    }

private:
    std::filesystem::path m_pathBase;
};
auto AddMesh(Document& document, BufferBuilder& bufferBuilder, const RawMesh& expMesh)
{
    string accessorIdIndices;
    string accessorIdPositions;
    string accessorIdNormals;
    string accessorIdTangents;
    string accessorIdTexCoord0;
    string accessorIdTexCoord1;
    string accessorIdTexCoord2;
    // Create a BufferView with a target of ELEMENT_ARRAY_BUFFER (as it will reference index
    // data) - it will be the 'current' BufferView that all the Accessors created by this
    // BufferBuilder will automatically reference
    bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

    // Add an Accessor for the indices
    std::vector<uint16_t> indices;

    for (int i = 0; i < expMesh.IndCount; i += 3)
    {
        indices.push_back(expMesh.indices[i + 1]);
        indices.push_back(expMesh.indices[i]);
        indices.push_back(expMesh.indices[i + 2]);
    }
    // Copy the Accessor's id - subsequent calls to AddAccessor may invalidate the returned reference
    accessorIdIndices = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT }).id;


    // Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
    
    // Add an Accessor for the positions
    std::vector<float> positions;
    for (int i = 0; i < expMesh.VertCount; i++)
    {
        positions.push_back(expMesh.vertices[i].X);
        positions.push_back(expMesh.vertices[i].Y);
        positions.push_back(expMesh.vertices[i].Z);
    }

    std::vector<float> minValues(3U, std::numeric_limits<float>::max());
    std::vector<float> maxValues(3U, std::numeric_limits<float>::lowest());

    const size_t positionCount = positions.size();

    // Accessor min/max properties must be set for vertex position data so calculate them here
    for (size_t i = 0U, j = 0U; i < positionCount; ++i, j = (i % 3U))
    {
        minValues[j] = std::min(positions[i], minValues[j]);
        maxValues[j] = std::max(positions[i], maxValues[j]);
    }

    accessorIdPositions = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT, false, std::move(minValues), std::move(maxValues) }).id;

    // Add an Accessor for the normals
    std::vector<float> normals;
    for (int i = 0; i < expMesh.VertCount; i++)
    {
        expMesh.normals[i].normalize();
        normals.push_back(expMesh.normals[i].X);
        normals.push_back(expMesh.normals[i].Y);
        normals.push_back(expMesh.normals[i].Z);
    }
    accessorIdNormals = bufferBuilder.AddAccessor(normals, { TYPE_VEC3, COMPONENT_FLOAT }).id;

    // Add an Accessor for the tangents
    std::vector<float> tangents;
    for (int i = 0; i < expMesh.VertCount; i++)
    {
        expMesh.tangents[i].normalize();
        tangents.push_back(expMesh.tangents[i].X);
        tangents.push_back(expMesh.tangents[i].Y);
        tangents.push_back(expMesh.tangents[i].Z);
        tangents.push_back(expMesh.tangents[i].W);
    }
    accessorIdTangents = bufferBuilder.AddAccessor(tangents, { TYPE_VEC4, COMPONENT_FLOAT }).id;


    // Add an Accessor for the tangents
    std::vector<float> texcoords0;
    for (int i = 0; i < expMesh.VertCount; i++)
    {
        texcoords0.push_back(expMesh.txcoord0[i].X);
        texcoords0.push_back(expMesh.txcoord0[i].Y);
    }
    accessorIdTexCoord0 = bufferBuilder.AddAccessor(texcoords0, { TYPE_VEC2, COMPONENT_FLOAT }).id;


    // Add an Accessor for the tangents
    std::vector<float> texcoords1;
    for (int i = 0; i < expMesh.VertCount; i++)
    {
        texcoords1.push_back(expMesh.txcoord1[i].X);
        texcoords1.push_back(expMesh.txcoord1[i].Y);
    }
    accessorIdTexCoord1 = bufferBuilder.AddAccessor(texcoords1, { TYPE_VEC2, COMPONENT_FLOAT }).id;

    std::vector<float> texcoords2;
    for (int i = 0; i < expMesh.VertCount; i++)
    {
        texcoords2.push_back(expMesh.txcoord2[i].X);
        texcoords2.push_back(expMesh.txcoord2[i].Y);
    }
    accessorIdTexCoord2 = bufferBuilder.AddAccessor(texcoords2, { TYPE_VEC2, COMPONENT_FLOAT }).id;
    // Create a very simple glTF Document with the following hierarchy:
    //  Scene
    //     Node
    //       Mesh (Triangle)
    //         MeshPrimitive
    //           Material (Blue)
    // 
    // A Document can be constructed top-down or bottom up. However, if constructed top-down
    // then the IDs of child entities must be known in advance, which prevents using the glTF
    // SDK's automatic ID generation functionality.

    // Construct a Material
    /*
    Material material;
    material.metallicRoughness.baseColorFactor = Color4(0.0f, 0.0f, 1.0f, 1.0f);
    material.metallicRoughness.metallicFactor = 0.2f;
    material.metallicRoughness.roughnessFactor = 0.4f;
    material.doubleSided = true;

    // Add it to the Document and store the generated ID
    //auto materialId = document.materials.Append(std::move(material), AppendIdPolicy::GenerateOnEmpty).id;
    */

    // Construct a MeshPrimitive. Unlike most types in glTF, MeshPrimitives are direct children
    // of their parent Mesh entity rather than being children of the Document. This is why they
    // don't have an ID member.
    MeshPrimitive meshPrimitive;
    //meshPrimitive.materialId = materialId;
    meshPrimitive.indicesAccessorId = accessorIdIndices;
    meshPrimitive.attributes[ACCESSOR_POSITION] = accessorIdPositions;
    meshPrimitive.attributes[ACCESSOR_NORMAL] = accessorIdNormals;
    meshPrimitive.attributes[ACCESSOR_TANGENT] = accessorIdTangents;
    meshPrimitive.attributes[ACCESSOR_TEXCOORD_0] = accessorIdTexCoord0;
    meshPrimitive.attributes[ACCESSOR_TEXCOORD_1] = accessorIdTexCoord1;
    meshPrimitive.attributes["TEXCOORD_2"] = accessorIdTexCoord2;
    // Construct a Mesh and add the MeshPrimitive as a child
    Mesh mesh;
    mesh.primitives.push_back(std::move(meshPrimitive));
    // Add it to the Document and store the generated ID
    auto meshId = document.meshes.Append(std::move(mesh), AppendIdPolicy::GenerateOnEmpty).id;

    // Construct a Node adding a reference to the Mesh
    Node node;
    node.meshId = meshId;
    // Add it to the Document and store the generated ID
    auto nodeId = document.nodes.Append(std::move(node), AppendIdPolicy::GenerateOnEmpty).id;

    return nodeId;
}
void WriteGLTF(const std::filesystem::path& path, const vector<RawMesh>& expMeshes)
{
    // Pass the absolute path, without the filename, to the stream writer
    auto streamWriter = std::make_unique<StreamWriter>(path.parent_path());

    std::filesystem::path pathFile = path.filename();
    std::filesystem::path pathFileExt = pathFile.extension();

    auto MakePathExt = [](const std::string& ext)
    {
        return "." + ext;
    };

    std::unique_ptr<ResourceWriter> resourceWriter;

    // If the file has a '.gltf' extension then create a GLTFResourceWriter
    if (pathFileExt == MakePathExt(GLTF_EXTENSION))
    {
        resourceWriter = std::make_unique<GLTFResourceWriter>(std::move(streamWriter));
    }

    // If the file has a '.glb' extension then create a GLBResourceWriter. This class derives
    // from GLTFResourceWriter and adds support for writing manifests to a GLB container's
    // JSON chunk and resource data to the binary chunk.
    if (pathFileExt == MakePathExt(GLB_EXTENSION))
    {
        resourceWriter = std::make_unique<GLBResourceWriter>(std::move(streamWriter));
    }

    if (!resourceWriter)
    {
        throw std::runtime_error("Command line argument path filename extension must be .gltf or .glb");
    }

    // The Document instance represents the glTF JSON manifest
    Document document;

    // Use the BufferBuilder helper class to simplify the process of
    // constructing valid glTF Buffer, BufferView and Accessor entities
    BufferBuilder bufferBuilder(std::move(resourceWriter));

    // Create all the resource data (e.g. triangle indices and
    // vertex positions) that will be written to the binary buffer
    const char* bufferId = nullptr;

    // Specify the 'special' GLB buffer ID. This informs the GLBResourceWriter that it should use
    // the GLB container's binary chunk (usually the desired buffer location when creating GLBs)
    if (dynamic_cast<const GLBResourceWriter*>(&bufferBuilder.GetResourceWriter()))
    {
        bufferId = GLB_BUFFER_ID;
    }

    // Create a Buffer - it will be the 'current' Buffer that all the BufferViews
    // created by this BufferBuilder will automatically reference
    bufferBuilder.AddBuffer(bufferId);

    // Construct a Scene
    Scene scene;
    for (int i = 0; i < expMeshes.size(); i++)
    {
        scene.nodes.push_back(AddMesh(document, bufferBuilder,expMeshes[i]));
    }
    // Add it to the Document, using a utility method that also sets the Scene as the Document's default
    document.SetDefaultScene(std::move(scene), AppendIdPolicy::GenerateOnEmpty);

    // Add all of the Buffers, BufferViews and Accessors that were created using BufferBuilder to
    // the Document. Note that after this point, no further calls should be made to BufferBuilder
    bufferBuilder.Output(document);

    std::string manifest;

    try
    {
        // Serialize the glTF Document into a JSON manifest
        manifest = Serialize(document, SerializeFlags::Pretty);
    }
    catch (const GLTFException& ex)
    {
        std::stringstream ss;

        ss << "Microsoft::glTF::Serialize failed: ";
        ss << ex.what();

        throw std::runtime_error(ss.str());
    }
    auto& gltfResourceWriter = bufferBuilder.GetResourceWriter();

    if (auto glbResourceWriter = dynamic_cast<GLBResourceWriter*>(&gltfResourceWriter))
    {
        glbResourceWriter->Flush(manifest, pathFile.string()); // A GLB container isn't created until the GLBResourceWriter::Flush member function is called
    }
    else
    {
        gltfResourceWriter.WriteExternal(pathFile.string(), manifest); // Binary resources have already been written, just need to write the manifest
    }
}