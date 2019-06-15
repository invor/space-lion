#include "gltfAssetComponentManager.hpp"

#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "tiny_gltf.h"

std::shared_ptr<tinygltf::Model> EngineCore::Graphics::Utility::loadGltfModel(std::string const & gltf_filepath)
{
    std::shared_ptr<tinygltf::Model> model = std::make_shared<tinygltf::Model>();
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    auto ret = loader.LoadASCIIFromFile(
        model.get(),
        &err,
        &warn,
        gltf_filepath
    );

    if (!warn.empty()) {
        std::cerr << "Warn: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "Err: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to parse glTF\n" << std::endl;
        return nullptr;
    }

    return model;
}
