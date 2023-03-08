#include "AnimationSystems.hpp"
#include <DirectXMath.h>

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
        size_t taggee_idx = transform_mngr.getIndex(cmp.taggee);
        glm::quat tagger_orientation = transform_mngr.getOrientation(tagger_idx);

        Vec3 tagger_position = transform_mngr.getPosition(tagger_idx);
        Vec3 taggee_position = transform_mngr.getPosition(taggee_idx);
        Vec3 rotated_offset = glm::rotate(tagger_orientation, cmp.offset);
        Vec3 tagger_front_pos = rotated_offset + tagger_position;

        // slowly approach the target location
        // xr::math::Pose::Slerp (without quaternion) slerp
        // TODO: prossibly also involve time dt
        float slerp_alpha = 0.1f;
        tagger_front_pos = taggee_position + (tagger_front_pos - taggee_position) * slerp_alpha;

        transform_mngr.setPosition(taggee_idx, tagger_front_pos);

        // possibly also set taggee orientation
    }
}


void EngineCore::Animation::tagHUD(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
    double dt)
{
    auto hud_cmps = tagalong_mngr.getHUDComponentDataCopy();

    for (auto& cmp : hud_cmps)
    {
        // if offset is replaced by distance to cam
        // then cmp.offset gets replaced by {0, 0, -1 * dist}
        // where the {0, 0, -1} denotes a position 1 meter in front of the camera

        size_t tagger_idx = transform_mngr.getIndex(cmp.tagger);
        size_t taggee_idx = transform_mngr.getIndex(cmp.taggee);
        glm::quat tagger_orientation = transform_mngr.getOrientation(tagger_idx);
        
        Vec3 tagger_position = transform_mngr.getPosition(tagger_idx);
        Vec3 rotated_offset = glm::rotate(tagger_orientation, cmp.offset);
        Vec3 tagger_front_pos = rotated_offset + tagger_position;

        transform_mngr.setPosition(taggee_idx, tagger_front_pos);
        
        if (false) {
            // TODO: need to flip y-axis for directx
            // this needs to be done exactly once, otherwise entity is constantly rotating
            tagger_orientation = glm::normalize(tagger_orientation * transform_mngr.getOrientation(taggee_idx));
        }
        // if orientation is set like this, the entity always stays the same
        //transform_mngr.setOrientation(taggee_idx, tagger_orientation);


        /*auto tagger_orientation = transform_mngr.getOrientation(tagger_idx);
        auto offset = DirectX::XMVECTOR({ 0.f, 0.f, -1.f });
        auto offset_quat = DirectX::XMVECTOR({ 0.f, 0.f, 0.f, 1.f });
        auto tagger_quat = DirectX::XMVECTOR({ tagger_orientation.x, tagger_orientation.y, tagger_orientation.z, tagger_orientation.w });
        // getPosition is equal to XrPose::position
        auto tp = transform_mngr.getPosition(tagger_idx);
        auto tagger_position = DirectX::XMVECTOR({ tp.x, tp.y, tp.z });
        auto one_meter_in_front_in_app_space = DirectX::XMVectorAdd(DirectX::XMVector3Rotate(offset, tagger_quat), tagger_position);
 
        
        Vec3 taggee_position = Vec3(DirectX::XMVectorGetX(one_meter_in_front_in_app_space), DirectX::XMVectorGetY(one_meter_in_front_in_app_space), DirectX::XMVectorGetZ(one_meter_in_front_in_app_space));
        transform_mngr.setPosition(taggee_idx, taggee_position);
        transform_mngr.setOrientation(taggee_idx, tagger_orientation);*/

        // TODO: calc correct posiition here, so, that target is always in front of camera/view
        // see https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/SampleSceneUwp/Scene_Orbit.cpp

        /*auto view_forward_in_app_space = DirectX::XMVectorSubtract(one_meter_in_front_in_app_space, tagger_position);
        auto view_forward_in_app_space_norm = DirectX::XMVector3Normalize(view_forward_in_app_space);
        auto scale = DirectX::XMVectorGetX(DirectX::XMVector3Dot(view_forward_in_app_space, { 0.f, -1.f, 0.f }));
        auto view_forward_in_gravity = DirectX::XMVectorScale(DirectX::XMVECTOR({ 0.f, -1.f, 0.f }), scale);
        auto eye_level_forward_in_app_space = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(view_forward_in_app_space, view_forward_in_gravity));
        auto entity_in_app_space = DirectX::XMVectorAdd(tagger_position, DirectX::XMVectorScale(view_forward_in_app_space_norm, 1.f));
        auto virtual_gaze_orientation = DirectX::XMMatrixLookToRH(entity_in_app_space, view_forward_in_app_space_norm, DirectX::XMVECTOR({ 0.f, 1.f, 0.f }));
        auto inv_gaze = DirectX::XMMatrixInverse(nullptr, virtual_gaze_orientation);
        DirectX::XMVECTOR sc, dx_o, dx_pos;
        DirectX::XMMatrixDecompose(&sc, &dx_o, &dx_pos, inv_gaze);
        Vec3 taggee_position = Vec3(DirectX::XMVectorGetX(dx_pos), DirectX::XMVectorGetY(dx_pos), DirectX::XMVectorGetZ(dx_pos));
        Quat taggee_orientation = Quat(DirectX::XMVectorGetX(dx_o), DirectX::XMVectorGetY(dx_o), DirectX::XMVectorGetZ(dx_o), DirectX::XMVectorGetW(dx_o));*/
    }
}