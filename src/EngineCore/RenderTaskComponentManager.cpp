/// <copyright file="RenderTaskComponentManager.cpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#include "RenderTaskComponentManager.hpp"

void EngineCore::Graphics::RenderTaskComponentManager::addComponent(
    Entity entity, 
    ResourceID mesh, 
    size_t mesh_component_subidx, 
    ResourceID shader_prgm, 
    size_t mtl_component_subidx, 
    size_t cached_transform_idx,
    size_t cached_mesh_idx,
    size_t cached_material_idx,
    bool visible)
{
    std::unique_lock<std::shared_mutex> lock(m_data_mutex);

    addIndex(entity.id(), m_data.size());

    m_data.emplace_back(Data(entity, mesh, mesh_component_subidx, shader_prgm, mtl_component_subidx, visible));

    m_data.back().cached_transform_idx = cached_transform_idx;
    m_data.back().cached_mesh_idx = cached_mesh_idx;
    m_data.back().cached_material_idx = cached_material_idx;

    std::sort(m_data.begin(), m_data.end());
}

std::vector<EngineCore::Graphics::RenderTaskComponentManager::Data>  & EngineCore::Graphics::RenderTaskComponentManager::getComponentData()
{
    return m_data;
}

std::vector<EngineCore::Graphics::RenderTaskComponentManager::Data> EngineCore::Graphics::RenderTaskComponentManager::getComponentDataCopy()
{
    std::vector<EngineCore::Graphics::RenderTaskComponentManager::Data> retval;

    {
        std::shared_lock<std::shared_mutex> lock(m_data_mutex);

        retval = m_data;
    }

    return retval;
}
