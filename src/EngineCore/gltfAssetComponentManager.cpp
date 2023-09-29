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


std::shared_ptr<tinygltf::Model> EngineCore::Graphics::GltfAssetComponentManager::addGltfModelToCache(
    std::string const& gltf_filepath)
{
    ModelPtr retval = nullptr;

    // check for model in cache
    {
        std::shared_lock<std::shared_mutex> lock(m_gltf_models_mutex);
        auto query = m_gltf_models.find(gltf_filepath);

        if (query != m_gltf_models.end()) {
            retval = query->second;
        }
    }

    // if model not found in cache, load and add now
    if (retval == nullptr) {
        retval = Utility::loadGltfModel(gltf_filepath);

        std::unique_lock<std::shared_mutex> lock(m_gltf_models_mutex);
        m_gltf_models.insert(std::make_pair(gltf_filepath, retval));
    }

    return retval;
}

void EngineCore::Graphics::GltfAssetComponentManager::addGltfModelToCache(
    std::string const& gltf_filepath,
    EngineCore::Graphics::GltfAssetComponentManager::ModelPtr const& gltf_model)
{
    {
        std::unique_lock<std::shared_mutex> lock(m_gltf_models_mutex);
        m_gltf_models.insert(std::make_pair(gltf_filepath, gltf_model));
    }
}

void EngineCore::Graphics::GltfAssetComponentManager::clearModelCache()
{
    std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
    std::unique_lock<std::shared_mutex> assets_lock(m_gltf_models_mutex);

    for (auto& asset : m_gltf_models) {
        asset.second.reset();
    }
}

void EngineCore::Graphics::GltfAssetComponentManager::addComponent(
    Entity entity,
    std::string const& gltf_filepath,
    size_t gltf_node_idx)
{
    size_t cmp_idx = m_data.size();
    addIndex(entity.id(), cmp_idx);

    std::unique_lock<std::shared_mutex> lock(m_data_mutex);
    m_data.push_back({ entity,gltf_filepath,gltf_node_idx });
}

std::vector<EngineCore::Graphics::GltfAssetComponentManager::ComponentData> EngineCore::Graphics::GltfAssetComponentManager::getComponents() const
{
    std::vector<EngineCore::Graphics::GltfAssetComponentManager::ComponentData> retval;
    {
        std::shared_lock<std::shared_mutex> lock(m_data_mutex);
        retval = m_data;
    }
    return retval;
}