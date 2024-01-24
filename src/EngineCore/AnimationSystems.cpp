#include "AnimationSystems.hpp"

void EngineCore::Animation::animateTurntables(
    EngineCore::Common::TransformComponentManager & transform_mngr,
    EngineCore::Animation::TurntableComponentManager & turntable_mngr,
    double dt,
    Utility::TaskScheduler& task_scheduler)
{
    auto t_0 = std::chrono::high_resolution_clock::now();

    std::vector<EngineCore::Animation::TurntableComponentManager::Data> tt_cmps = turntable_mngr.getComponentDataCopy();

    std::vector<std::pair<size_t, size_t>> from_to_pairs;
    size_t bucket_cnt = 6;
    for (size_t i = 0; i < bucket_cnt; ++i) {
        from_to_pairs.push_back({ tt_cmps.size() * (float(i) / float(bucket_cnt)), tt_cmps.size() * (float(i + 1) / float(bucket_cnt)) });
    }
    
    for (auto from_to : from_to_pairs) {
        task_scheduler.submitTask(
            [&transform_mngr, &tt_cmps, from_to, dt]() {
                for (size_t i = from_to.first; i < from_to.second; ++i)
                {
                    auto transform_idx = transform_mngr.getIndex((tt_cmps)[i].entity);
                    transform_mngr.rotateLocal(transform_idx, glm::angleAxis(static_cast<float>((tt_cmps)[i].angle * dt), (tt_cmps)[i].axis));
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
    double dt) 
{
    auto tag_cmps = tagalong_mngr.getTagComponentDataCopy();

    for (auto& cmp : tag_cmps)
    {
        size_t target_idx = transform_mngr.getIndex(cmp.target);
        Vec3 target_position = transform_mngr.getPosition(target_idx);
        glm::quat target_orientation = transform_mngr.getOrientation(target_idx);
        Vec3 rotated_offset = glm::rotate(target_orientation, cmp.offset);
        Vec3 target_front_pos = rotated_offset + target_position;

        size_t entity_idx = transform_mngr.getIndex(cmp.entity);
        Vec3 entity_position = transform_mngr.getPosition(entity_idx);

        Vec3 movement_vector = target_front_pos - entity_position;
        float deadzone_factor = std::max(0.0f,(glm::length(movement_vector) - cmp.deadzone)) / glm::length(movement_vector);

        target_front_pos = entity_position + movement_vector * deadzone_factor * std::min(1.0f, cmp.slerp_alpha * (static_cast<float>(dt)/0.1f));

        transform_mngr.setPosition(entity_idx, target_front_pos);
    }
}


void EngineCore::Animation::animateBillboards(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::BillboardComponentManager& billboard_mngr,
    double dt)
{
    auto billboard_cmps = billboard_mngr.getBillboardComponentDataCopy();

    for (auto& cmp : billboard_cmps) {
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
        auto mirrored_target_pos = target_pos - 2.0f * (target_pos-entity_pos);
        auto r = glm::toQuat(glm::inverse(glm::lookAt(entity_pos, mirrored_target_pos, Vec3(0.0f, 1.0f, 0.0f))));
        transform_mngr.setOrientation(entity_idx, r);
    }
}