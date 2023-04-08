#include "BillboardComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"

using namespace EngineCore::Animation;

void BillboardComponentManager::addComponent(Entity entity, Entity target)
{
    std::unique_lock<std::shared_mutex> lock(m_billboard_data_access_mutex);

    uint idx = static_cast<uint>(m_billboard_data.size());

    addIndex(entity.id(), idx);

    m_billboard_data.push_back(Data(entity, target));
}

std::vector<BillboardComponentManager::Data> BillboardComponentManager::getBillboardComponentDataCopy()
{
    std::vector<BillboardComponentManager::Data> retval;

    {
        std::shared_lock<std::shared_mutex> lock(m_billboard_data_access_mutex);

        retval = m_billboard_data;
    }

    return retval;
}
