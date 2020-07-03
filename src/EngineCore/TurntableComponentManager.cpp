#include "TurntableComponentManager.hpp"

#include "TransformComponentManager.hpp"
#include "types.hpp"
#include "WorldState.hpp"


void EngineCore::Animation::TurntableComponentManager::addComponent(Entity entity, float angle, Vec3 axis)
{
    std::unique_lock<std::shared_mutex> lock(m_dataAccess_mutex);

    uint idx = static_cast<uint>(m_data.size());

    addIndex(entity.id(), idx);

    m_data.push_back(Data(entity, angle, axis));
}

//void EngineCore::Animation::TurntableComponentManager::animate(double dt)
//{
//    std::unique_lock<std::mutex> lock(m_dataAccess_mutex);
//
//    for (auto& cmp : m_data)
//    {
//        //auto& transform_mngr = m_world.accessTransformManager();
//        auto& transform_mngr = m_world.get<EngineCore::Common::TransformComponentManager>();
//
//        auto query = transform_mngr.getIndex(cmp.entity);
//
//        if (!query.empty())
//        {
//            transform_mngr.rotateLocal(query.front(), glm::angleAxis(static_cast<float>(cmp.angle * dt) ,cmp.axis));
//        }
//    }
//}

std::vector<EngineCore::Animation::TurntableComponentManager::Data> EngineCore::Animation::TurntableComponentManager::getComponentDataCopy()
{
    std::vector<EngineCore::Animation::TurntableComponentManager::Data> retval;

    {
        std::shared_lock<std::shared_mutex> lock(m_dataAccess_mutex);

        retval = m_data;
    }

    return retval;
}
