#include "TriggerSystems.hpp"

#include "utility.hpp"

void EngineCore::Common::checkProximityTriggers(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Common::ProximityTriggerComponentManager& proximity_trigger_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{
    size_t component_cnt = proximity_trigger_mngr.getComponentCount();

    std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(component_cnt, 6);

    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&transform_mngr, &proximity_trigger_mngr, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    if (proximity_trigger_mngr.checkComponent(i)) {
                        auto& cmp = proximity_trigger_mngr.getComponent(i);

                        auto entity_transform_idx = transform_mngr.getIndex(cmp.entity);
                        auto target_transform_idx = transform_mngr.getIndex(cmp.target);

                        float distance = glm::length(transform_mngr.getWorldPosition(entity_transform_idx) - transform_mngr.getWorldPosition(target_transform_idx));

                        if (distance < cmp.trigger_distance && !cmp.in_proximity) {
                            cmp.enter_callback();
                            cmp.in_proximity = true;
                        }
                        else if (distance > cmp.trigger_distance && cmp.in_proximity) {
                            cmp.leave_callback();
                            cmp.in_proximity = false;
                        }
                    }
                }
            }
        );
    }

    task_scheduler.waitWhileBusy();
}

void EngineCore::Common::updateCooldownTriggers(
    EngineCore::Common::CooldownTriggerComponentManager& cooldown_trigger_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{
    size_t component_cnt = cooldown_trigger_mngr.getComponentCount();

    std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(component_cnt, 6);

    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&cooldown_trigger_mngr, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    if (cooldown_trigger_mngr.checkComponent(i)) {
                        auto& cmp = cooldown_trigger_mngr.getComponent(i);
                        if (cmp.is_active) {
                            cmp.remaining_time -= static_cast<float>(dt);

                            if (cmp.remaining_time <= 0.0f) {
                                cmp.cooldown_callback();
                                cmp.is_active = false;
                            }
                        }
                    }
                }
            }
        );
    }

    task_scheduler.waitWhileBusy();
}
