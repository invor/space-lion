#include "TurntableComponentManager.hpp"

#include "TransformComponentManager.hpp"
#include "types.hpp"
#include "WorldState.hpp"

EngineCore::Animation::TurntableComponentManager::TurntableComponentManager(WorldState & world)
    : m_world(world)
{
}

void EngineCore::Animation::TurntableComponentManager::addComponent(Entity entity, float angle, Vec3 axis)
{
    std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

    uint idx = static_cast<uint>(m_data.size());

    addIndex(entity.id(), idx);

    m_data.push_back(Data(entity, angle, axis));
}

void EngineCore::Animation::TurntableComponentManager::animate(double dt)
{
    std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

    for (auto& cmp : m_data)
    {
         auto& transform_mngr = m_world.accessTransformManager();

         auto query = transform_mngr.getIndex(cmp.entity);

         if (!query.empty())
         {
             transform_mngr.rotateLocal(query.front(), glm::angleAxis(static_cast<float>(cmp.angle * dt) ,cmp.axis));
         }
    }
}
