/// <copyright file="MaterialComponentManager.cpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#include "MaterialComponentManager.hpp"

size_t EngineCore::Graphics::MaterialComponentManager::addComponent(
    Entity entity,
    std::string material_name,
    ResourceID shader_program)
{
    return addComponent(
        entity,
        material_name,
        shader_program,
        std::array<float, 4>{1.0f, 0.5f, 1.0f, 1.0f},
        std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f},
        0.8f,
        std::vector<std::pair<MaterialComponentData::TextureSemantic, ResourceID>>());
}

size_t EngineCore::Graphics::MaterialComponentManager::addComponent(
    Entity entity,
    std::string material_name,
    ResourceID shader_program,
    std::array<float, 4> albedo_colour,
    std::array<float, 4> specular_colour,
    float roughness)
{
    return addComponent(
        entity,
        material_name,
        shader_program,
        albedo_colour,
        specular_colour,
        roughness,
        std::vector<std::pair<MaterialComponentData::TextureSemantic, ResourceID>>());
}

std::vector<EngineCore::Graphics::ResourceID> EngineCore::Graphics::MaterialComponentManager::getTextures(
    size_t idx, 
    MaterialComponentData::TextureSemantic semantic) const
{
    std::vector<ResourceID> retval;

    auto [page_idx, idx_in_page] = data_.getIndices(idx);
    for (auto& tx : data_(page_idx, idx_in_page).textures)
    {
        if (std::get<0>(tx) == semantic)
        {
            retval.push_back(std::get<1>(tx));
        }
    }

    return retval;
}