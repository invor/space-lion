#include "AnimationSystems.hpp"

void EngineCore::Animation::animateTurntables(
    EngineCore::Common::TransformComponentManager & transform_mngr,
    EngineCore::Animation::TurntableComponentManager & turntable_mngr,
    double dt)
{
    auto& tt_cmps = turntable_mngr.accessComponents();

    for (auto& cmp : tt_cmps)
    {
        auto query = transform_mngr.getIndex(cmp.entity);

        if (!query.empty())
        {
            transform_mngr.rotateLocal(query.front(), glm::angleAxis(static_cast<float>(cmp.angle * dt), cmp.axis));
        }
    }
}
