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

        size_t tagger_idx = transform_mngr.getIndex(cmp.tagger);
        Vec3 tagger_position = transform_mngr.getPosition(tagger_idx);
        glm::quat tagger_orientation = transform_mngr.getOrientation(tagger_idx);
        Vec3 rotated_offset = glm::rotate(tagger_orientation, cmp.offset);
        Vec3 tagger_front_pos = rotated_offset + tagger_position;

        size_t taggee_idx = transform_mngr.getIndex(cmp.taggee);
        Vec3 taggee_position = transform_mngr.getPosition(taggee_idx);

        tagger_front_pos = taggee_position + (tagger_front_pos - taggee_position) * cmp.slerp_alpha;

        transform_mngr.setPosition(taggee_idx, tagger_front_pos);
    }
}


void EngineCore::Animation::billboard(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::BillboardComponentManager& billbaord_mngr,
    double dt)
{
    auto billboard_cmps = billbaord_mngr.getBillboardComponentDataCopy();

    for (auto& cmp : billboard_cmps) {
        size_t tagger_idx = transform_mngr.getIndex(cmp.tagger);
        glm::quat tagger_orientation = transform_mngr.getOrientation(tagger_idx);
        size_t taggee_idx = transform_mngr.getIndex(cmp.taggee);
        transform_mngr.setOrientation(taggee_idx, tagger_orientation);
    }
}