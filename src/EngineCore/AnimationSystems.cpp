#include "AnimationSystems.hpp"

#include "utility.hpp"
#include <chrono>

void EngineCore::Animation::animateTurntables(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::TurntableComponentManager& turntable_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{
    auto t_0 = std::chrono::high_resolution_clock::now();

    size_t component_cnt = turntable_mngr.getComponentCount();

    std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(component_cnt,6);
    
    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&transform_mngr, &turntable_mngr, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    if (turntable_mngr.checkComponent(i)) {
                        auto const& cmp = turntable_mngr.getComponent(i);

                        auto transform_idx = transform_mngr.getIndex(cmp.entity);
                        transform_mngr.rotateLocal(transform_idx, glm::angleAxis(static_cast<float>(cmp.angle * dt), cmp.axis));
                    }
                }
            }
        );
    }

    task_scheduler.waitWhileBusy();

    auto t_1 = std::chrono::high_resolution_clock::now();

    auto dt2 = std::chrono::duration_cast<std::chrono::duration<double>>(t_1 - t_0).count();
    std::cout << "Animation system computation time: " << dt2 << std::endl;
}


void EngineCore::Animation::animateTagAlong(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{
    size_t component_cnt = tagalong_mngr.getComponentCount();

    std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(component_cnt, 6);

    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&transform_mngr, &tagalong_mngr, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    if (tagalong_mngr.checkComponent(i)) {
                        auto const& cmp = tagalong_mngr.getComponent(i);

                        size_t target_idx = transform_mngr.getIndex(cmp.target);
                        Mat4x4 target_xform = transform_mngr.getWorldTransformation(target_idx);
                        Vec3 target_front_pos = Vec3(target_xform * Vec4(cmp.offset, 1.0f));
                        
                        size_t entity_idx = transform_mngr.getIndex(cmp.entity);
                        Vec3 entity_position = transform_mngr.getWorldPosition(entity_idx);
                        
                        Vec3 movement_vector = target_front_pos - entity_position;
                        float distance = glm::length(movement_vector);
                        float deadzone_factor = distance > cmp.deadzone ? (distance - cmp.deadzone) / distance : 0.0f;
                        
                        target_front_pos = entity_position + movement_vector * deadzone_factor * std::min(1.0f, (static_cast<float>(dt) / cmp.time_to_target));
                        
                        transform_mngr.setPosition(entity_idx, target_front_pos);
                    }
                }
            }
        );
    }

    task_scheduler.waitWhileBusy();
}


void EngineCore::Animation::animateBillboards(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::BillboardComponentManager& billboard_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{

    size_t component_cnt = billboard_mngr.getComponentCount();

    std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(component_cnt, 6);

    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&transform_mngr, &billboard_mngr, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    if (billboard_mngr.checkComponent(i)) {
                        auto const& cmp = billboard_mngr.getComponent(i);

                        size_t target_idx = transform_mngr.getIndex(cmp.target);
                        size_t entity_idx = transform_mngr.getIndex(cmp.entity);

                        Vec3 target_pos = transform_mngr.getPosition(target_idx);
                        Vec3 entity_pos = transform_mngr.getPosition(entity_idx);

                        Entity parent = transform_mngr.getParent(entity_idx);
                        if (parent != EntityManager::invalidEntity())
                        {
                            Mat4x4 to_parent_space = glm::inverse(transform_mngr.getWorldTransformation(transform_mngr.getIndex(parent)));
                            target_pos = Vec3(to_parent_space * Vec4(target_pos, 1.0f));
                        }

                        // Mirror target position to make +z face the original target
                        auto mirrored_target_pos = target_pos - 2.0f * (target_pos - entity_pos);
                        auto r = glm::toQuat(glm::inverse(glm::lookAt(entity_pos, mirrored_target_pos, Vec3(0.0f, 1.0f, 0.0f))));
                        transform_mngr.setOrientation(entity_idx, r);
                    }
                }
            }
        );
    }

    task_scheduler.waitWhileBusy();
}

void EngineCore::Animation::animatioMoveTo(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::MoveToComponentManager& moveto_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{
    size_t component_cnt = moveto_mngr.getComponentCount();

    std::vector<std::pair<size_t, size_t>> from_to_pairs = utility::buildComponentProcessingRanges(component_cnt, 6);

    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&transform_mngr, &moveto_mngr, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    if (moveto_mngr.checkComponent(i)) {
                        auto const& cmp = moveto_mngr.getComponent(i);

                        auto transform_idx = transform_mngr.getIndex(cmp.entity);

                        if (cmp.move_orientation == Space::LOCAL)
                        {
                            auto current_position = transform_mngr.getPosition(transform_idx);
                            auto move_direction = cmp.target_position - current_position;
                            auto distance = glm::length(move_direction);
                            if (distance > std::numeric_limits<float>::epsilon())
                            {
                                move_direction /= distance;
                                transform_mngr.translate(transform_idx, move_direction * std::min(distance, static_cast<float>(cmp.speed * dt)));
                            }
                        }
                        else
                        {
                            //TODO
                        }
                    }
                }
            }
        );
    }

    task_scheduler.waitWhileBusy();
}
