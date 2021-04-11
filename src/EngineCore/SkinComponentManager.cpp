#include "SkinComponentManager.hpp"

void EngineCore::Animation::SkinComponentManager::addComponent(Entity entity, std::vector<Entity> const& joints, std::vector<Mat4x4> const& inverse_bind_matrices)
{
    std::unique_lock<std::shared_mutex> lock(m_dataAccess_mutex);

    uint idx = static_cast<uint>(m_data.size());

    addIndex(entity.id(), idx);

    m_data.push_back(Data(entity, joints, inverse_bind_matrices));
}

std::vector<Entity> const& EngineCore::Animation::SkinComponentManager::getJoints(Entity entity)
{
    return getJoints(getIndex(entity.id()));
}

std::vector<Entity> const& EngineCore::Animation::SkinComponentManager::getJoints(size_t component_idx)
{
    return m_data[component_idx].joints;
}

std::vector<Mat4x4> const& EngineCore::Animation::SkinComponentManager::getInvsereBindMatrices(Entity entity)
{
    return getInvsereBindMatrices(getIndex(entity.id()));
}

std::vector<Mat4x4> const& EngineCore::Animation::SkinComponentManager::getInvsereBindMatrices(size_t component_idx)
{
    return m_data[component_idx].inverse_bind_matrices;
}
