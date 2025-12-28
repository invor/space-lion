#include "CooldownTriggerComponentManager.hpp"

size_t EngineCore::Common::CooldownTriggerComponentManager::addComponent(Entity entity, float reset_time, std::function<void()> cooldown_callback, bool start_on_creation)
{
    auto index = data_.addComponent(
        {
            entity,
            start_on_creation ? true : false,
            reset_time,
            reset_time,
            cooldown_callback,
        }
    );

    addIndex(entity.id(), index);

    auto [page_idx, idx_in_page] = data_.getIndices(index);

    return index;
}

void EngineCore::Common::CooldownTriggerComponentManager::resetCooldown(Entity entity)
{
    auto indices = getIndex(entity);

    for (auto index : indices) {
        resetCooldown(index);
    }
}

void EngineCore::Common::CooldownTriggerComponentManager::resetCooldown(size_t index)
{
    auto indices = data_.getIndices(index);
    if (data_.checkComponent(indices.first, indices.second)) {
        data_(indices.first, indices.second).remaining_time = data_(indices.first, indices.second).reset_time;
        data_(indices.first, indices.second).is_active = true;
    }
}
