/// <copyright file="MaterialComponentManager.cpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#include "MaterialComponentManager.hpp"

void EngineCore::Graphics::MaterialComponentManager::addComponent(
    Entity entity,
    std::string material_name,
    ResourceID shader_program)
{
    addComponent(
        entity,
        material_name,
        shader_program,
        std::array<float, 4>{1.0f, 0.5f, 1.0f, 1.0f},
        std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f},
        0.8f,
        std::vector<std::pair<TextureSemantic, ResourceID>>());
}

void EngineCore::Graphics::MaterialComponentManager::addComponent(
    Entity entity,
    std::string material_name,
    ResourceID shader_program,
    std::array<float, 4> albedo_colour,
    std::array<float, 4> specular_colour,
    float roughness)
{
    addComponent(
        entity,
        material_name,
        shader_program,
        albedo_colour,
        specular_colour,
        roughness,
        std::vector<std::pair<TextureSemantic, ResourceID>>());
}

EngineCore::Graphics::ResourceID EngineCore::Graphics::MaterialComponentManager::getTextures(size_t component_idx, TextureSemantic semantic) const
{
    std::shared_lock<std::shared_mutex> lock(m_data_mutex);

    //std::vector<ResourceID> retval;
    //
    //auto range_query = m_component_data[component_idx].textures.equal_range(semantic);
    //
    //if (range_query.first != m_component_data[component_idx].textures.end())
    //{
    //    for (auto itr = range_query.first; itr != range_query.second; ++itr){
    //        retval.push_back(itr->second);
    //    }
    //}
    //
    //return retval;

    ResourceID retval;

    for (auto& tx : m_component_data[component_idx].textures)
    {
        if (std::get<0>(tx) == semantic)
        {
            retval = std::get<1>(tx);
            break;
        }
    }

    return retval;
}