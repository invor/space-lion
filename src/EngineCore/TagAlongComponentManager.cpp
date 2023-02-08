#include "TagAlongComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"


void EngineCore::Animation::TagAlongComponentManager::addComponent(Entity entity, Entity target, Vec3 offset)
{
    std::unique_lock<std::shared_mutex> lock(m_dataAccess_mutex);

    uint idx = static_cast<uint>(m_data.size());

    addIndex(entity.id(), idx);

    m_data.push_back(Data(entity, target, offset));
}


std::vector<EngineCore::Animation::TagAlongComponentManager::Data> EngineCore::Animation::TagAlongComponentManager::getComponentDataCopy()
{
    std::vector<EngineCore::Animation::TagAlongComponentManager::Data> retval;

    {
        std::shared_lock<std::shared_mutex> lock(m_dataAccess_mutex);

        retval = m_data;
    }

    return retval;
}
