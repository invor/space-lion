#include "MoveToComponentManager.hpp"

size_t EngineCore::Animation::MoveToComponentManager::addComponent(Entity entity, Vec3 target_position, float speed, Space move_orientation)
{
    auto index = data_.addComponent(
        {
            entity,
            target_position,
            speed,
            move_orientation
        }
    );

    addIndex(entity.id(), index);

    return index;
}

void EngineCore::Animation::MoveToComponentManager::setTargetPosition(Entity entity, Vec3 target_position)
{
    auto idx = getIndex(entity.id());

    setTargetPosition(idx, target_position);
}

void EngineCore::Animation::MoveToComponentManager::setTargetPosition(size_t index, Vec3 target_position)
{
    auto indices = data_.getIndices(index);

    data_(indices.first, indices.second).target_position = target_position;
}
