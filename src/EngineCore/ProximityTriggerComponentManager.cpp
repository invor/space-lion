#include "ProximityTriggerComponentManager.hpp"

size_t EngineCore::Common::ProximityTriggerComponentManager::addComponent(Entity entity, Entity target, float trigger_distance, std::function<void()> enter_callback, std::function<void()> leave_callback)
{
    auto index = data_.addComponent(
        {
            entity,
            target,
            trigger_distance,
            false,
            enter_callback,
            leave_callback
        }
    );

    addIndex(entity.id(), index);

    auto [page_idx, idx_in_page] = data_.getIndices(index);

    return index;
}