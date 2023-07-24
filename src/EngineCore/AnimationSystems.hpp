#ifndef AnimationSystems_hpp
#define AnimationSystems_hpp


#include "TransformComponentManager.hpp"
#include "TurntableComponentManager.hpp"
#include "TagAlongComponentManager.hpp"
#include "BillboardComponentManager.hpp"

namespace EngineCore {
namespace Animation {

    void animateTurntables(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TurntableComponentManager& turntable_mngr,
        double dt);

    void animateTagAlong(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
        double dt);

    void animateBillboards(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::BillboardComponentManager& billboard_mngr,
        double dt);
}
}

#endif // !AnimationSystems_hpp
