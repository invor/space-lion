#ifndef AnimationSystems_hpp
#define AnimationSystems_hpp

#include "BillboardComponentManager.hpp"
#include "MoveToComponentManager.hpp"
#include "TagAlongComponentManager.hpp"
#include "TaskScheduler.hpp"
#include "TransformComponentManager.hpp"
#include "TurntableComponentManager.hpp"

namespace EngineCore {
namespace Animation {

    void animateTurntables(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TurntableComponentManager& turntable_mngr,
        double dt,
        Utility::TaskScheduler& task_scheduler);

    void animateTagAlong(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
        double dt,
        Utility::TaskScheduler& task_scheduler);

    void animateBillboards(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::BillboardComponentManager& billboard_mngr,
        double dt,
        Utility::TaskScheduler& task_scheduler);

    void animatioMoveTo(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::MoveToComponentManager& moveto_mngr,
        double dt,
        Utility::TaskScheduler& task_scheduler);
}
}

#endif // !AnimationSystems_hpp
