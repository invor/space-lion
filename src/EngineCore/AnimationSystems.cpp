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
        transform_mngr.rotateLocal(transform_idx, glm::angleAxis(static_cast<float>(cmp.angle * dt), cmp.axis));
    }

    auto t_1 = std::chrono::high_resolution_clock::now();

    auto dt2 = std::chrono::duration_cast<std::chrono::duration<double>>(t_1 - t_0).count();
    //std::cout << "Animation system computation time: " << dt2 << std::endl;
}
