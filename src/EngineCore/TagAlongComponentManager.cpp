#include "TagAlongComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"

using namespace EngineCore::Animation;

void TagAlongComponentManager::addComponent(Entity entity, Entity target, Vec3 offset, float slerp_alpha)
{
    std::unique_lock<std::shared_mutex> lock(m_tag_data_access_mutex);

    uint idx = static_cast<uint>(m_tag_data.size());

    addIndex(entity.id(), idx);

    slerp_alpha = std::clamp(slerp_alpha, 0.f, 1.f);
    m_tag_data.push_back(Data(entity, target, offset, slerp_alpha));
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
