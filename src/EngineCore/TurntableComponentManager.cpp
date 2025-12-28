#include "TurntableComponentManager.hpp"

#include "TransformComponentManager.hpp"
#include "types.hpp"
#include "WorldState.hpp"


size_t EngineCore::Animation::TurntableComponentManager::addComponent(Entity entity, float angle, Vec3 axis)
{
    auto index = data_.addComponent(
        {
            entity,
            angle,
            axis
        }
    );

    addIndex(entity.id(), index);

    return index;
}
