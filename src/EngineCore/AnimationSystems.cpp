#include "AnimationSystems.hpp"

void EngineCore::Animation::animateTurntables(
    EngineCore::Common::TransformComponentManager & transform_mngr,
    EngineCore::Animation::TurntableComponentManager & turntable_mngr,
    double dt)
{
    auto t_0 = std::chrono::high_resolution_clock::now();

    auto tt_cmps = turntable_mngr.getComponentDataCopy();

    for (auto& cmp : tt_cmps)
    {
        auto transform_idx = transform_mngr.getIndex(cmp.entity);
        // if ( idx == max )
        transform_mngr.rotateLocal(transform_idx, glm::angleAxis(static_cast<float>(cmp.angle * dt), cmp.axis));
    }

    auto t_1 = std::chrono::high_resolution_clock::now();

    auto dt2 = std::chrono::duration_cast<std::chrono::duration<double>>(t_1 - t_0).count();
    //std::cout << "Animation system computation time: " << dt2 << std::endl;
}


void EngineCore::Animation::tagAlong(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
    double dt) 
{
    auto tag_cmps = tagalong_mngr.getTagComponentDataCopy();

    for (auto& cmp : tag_cmps)
    {
        // if offset is replaced by distance to cam
        // then cmp.offset gets replaced by {0, 0, -1 * dist}
        // where the {0, 0, -1} denotes a position 1 meter in front of the camera

        size_t target_idx = transform_mngr.getIndex(cmp.target);
        Vec3 target_position = transform_mngr.getPosition(target_idx);
        glm::quat target_orientation = transform_mngr.getOrientation(target_idx);
        Vec3 rotated_offset = glm::rotate(target_orientation, cmp.offset);
        Vec3 target_front_pos = rotated_offset + target_position;

        size_t entity_idx = transform_mngr.getIndex(cmp.entity);
        Vec3 entity_position = transform_mngr.getPosition(entity_idx);

        target_front_pos = entity_position + (target_front_pos - entity_position) * cmp.slerp_alpha;

        transform_mngr.setPosition(entity_idx, target_front_pos);
    }
}


void EngineCore::Animation::billboard(
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

        Vec3 up = Vec3(0, 1, 0);

        auto r = glm::toQuat(glm::inverse(glm::lookAt(entity_pos, target_pos, up)));
        //Quat r = Quat(1, 0, 0, 0);
        transform_mngr.setOrientation(entity_idx, r);
    }
}