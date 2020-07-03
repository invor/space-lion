/// <copyright file="RenderTaskComponentManager.cpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#include "RenderTaskComponentManager.hpp"

void EngineCore::Graphics::RenderTaskComponentManager::addComponent(Entity entity, ResourceID mesh, size_t mesh_component_subidx, ResourceID shader_prgm, size_t mtl_component_subidx, bool visible)
{
    std::unique_lock<std::shared_mutex> lock(m_data_mutex);

    addIndex(entity.id(), m_data.size());

    m_data.emplace_back(Data(entity, mesh, mesh_component_subidx, shader_prgm, mtl_component_subidx, visible));
    std::sort(m_data.begin(), m_data.end());
}

std::vector<EngineCore::Graphics::RenderTaskComponentManager::Data> const & EngineCore::Graphics::RenderTaskComponentManager::getComponentData()
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
