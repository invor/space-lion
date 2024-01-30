#include "TagAlongComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"

using namespace EngineCore::Animation;

void TagAlongComponentManager::addComponent(Entity entity, Entity target, Vec3 offset, float time_to_target, float deadzone)
{
    std::unique_lock<std::shared_mutex> lock(m_tag_data_access_mutex);

    size_t idx = m_tag_data.size();

    addIndex(entity.id(), idx);

    m_tag_data.push_back(Data(entity, target, offset, time_to_target, deadzone));
}

void EngineCore::Animation::TagAlongComponentManager::setTarget(Entity entity, Entity target)
{
    std::unique_lock<std::shared_mutex> lock(m_tag_data_access_mutex);

    size_t idx = getIndex(entity);

    m_tag_data[idx].target = target;
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
