#ifndef AnimationSystems_hpp
#define AnimationSystems_hpp


#include "TransformComponentManager.hpp"
#include "TurntableComponentManager.hpp"

namespace EngineCore {
namespace Animation {

    void animateTurntables(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TurntableComponentManager& turntable_mngr,
        double dt);

}
}

#endif // !AnimationSystems_hpp
