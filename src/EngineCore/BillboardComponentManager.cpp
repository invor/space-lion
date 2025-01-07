#include "BillboardComponentManager.hpp"

#include "types.hpp"
#include "WorldState.hpp"

using namespace EngineCore::Animation;

size_t BillboardComponentManager::addComponent(Entity entity, Entity target)
{
    auto index = data_.addComponent(
        {
            entity,
            target
        }
    );

    addIndex(entity.id(), index);

    return index;
}
