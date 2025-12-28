#include "TagAlongComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"

using namespace EngineCore::Animation;

size_t TagAlongComponentManager::addComponent(Entity entity, Entity target, Vec3 offset, float time_to_target, float deadzone)
{
    auto index = data_.addComponent(
        {
            entity,
            target,
            offset,
            time_to_target,
            deadzone
        }
    );

    addIndex(entity.id(), index);

    return index;
}

void EngineCore::Animation::TagAlongComponentManager::setTarget(Entity entity, Entity target)
{
    size_t idx = getIndex(entity);
    auto indices = data_.getIndices(idx);

    data_(indices.first, indices.second).target = target;
}
