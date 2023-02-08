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


// TODO: see https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/SampleSceneUwp/Scene_Orbit.cpp
// using slerp for movement to target location
void EngineCore::Animation::tagAlong(
    EngineCore::Common::TransformComponentManager& transform_mngr,
    EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
    double dt) 
{
    auto ta_cmps = tagalong_mngr.getComponentDataCopy();

    for (auto& cmp : ta_cmps) 
    {
        auto cmp_idx = transform_mngr.getIndex(cmp.entity);
        Vec3 cmp_position = transform_mngr.getWorldPosition(cmp_idx);

        auto target_idx = transform_mngr.getIndex(cmp.target);
        Vec3 target_position = transform_mngr.getWorldPosition(target_idx) + cmp.offset;

        float speed = (float)dt / 20.f; // 20 seconds per meter
        Vec3 diff = target_position - cmp_position;
        float dist = length(diff);

        if (dist > 1e-5) {
            Vec3 direction = normalize(diff);
            Vec3 stride = speed * direction;
            float stride_length = length(stride);

            transform_mngr.translate(cmp_idx, (stride_length > dist ? diff : stride));
        }
    }
}
