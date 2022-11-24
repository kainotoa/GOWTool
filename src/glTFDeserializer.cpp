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