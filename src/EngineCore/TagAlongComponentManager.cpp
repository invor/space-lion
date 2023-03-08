#include "TagAlongComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"

using namespace EngineCore::Animation;

void TagAlongComponentManager::addTagComponent(Entity entity, Entity target, Vec3 offset)
{
    std::unique_lock<std::shared_mutex> lock(m_tag_data_access_mutex);

    uint idx = static_cast<uint>(m_tag_data.size());

    addIndex(entity.id(), idx);

    m_tag_data.push_back(Data(entity, target, offset));
}

void TagAlongComponentManager::addHUDComponent(Entity entity, Entity target, Vec3 offset)
{
    std::unique_lock<std::shared_mutex> lock(m_hud_data_access_mutex);

    uint idx = static_cast<uint>(m_hud_data.size());

    addIndex(entity.id(), idx);

    m_hud_data.push_back(Data(entity, target, offset));
}

std::vector<TagAlongComponentManager::Data> TagAlongComponentManager::getTagComponentDataCopy()
{
    std::vector<TagAlongComponentManager::Data> retval;

    {
        std::shared_lock<std::shared_mutex> lock(m_tag_data_access_mutex);

        retval = m_tag_data;
    }

    return retval;
}

std::vector<TagAlongComponentManager::Data> TagAlongComponentManager::getHUDComponentDataCopy()
{
    std::vector<TagAlongComponentManager::Data> retval;

    {
        std::shared_lock<std::shared_mutex> lock(m_hud_data_access_mutex);

        retval = m_hud_data;
    }

    return retval;
}
