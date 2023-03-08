#ifndef AnimationSystems_hpp
#define AnimationSystems_hpp


#include "TransformComponentManager.hpp"
#include "TurntableComponentManager.hpp"
#include "TagAlongComponentManager.hpp"

namespace EngineCore {
namespace Animation {

    void animateTurntables(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TurntableComponentManager& turntable_mngr,
        double dt);

    // A target entity 'taggee' slowly approaches a target entity 'tagger'
    void tagAlong(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
        double dt);

    // Basically same as tagAlong, but instant without slow approaching the tagger
    // This can also be done by setting the taggee as child entity of the tagger
    void tagHUD(
        EngineCore::Common::TransformComponentManager& transform_mngr,
        EngineCore::Animation::TagAlongComponentManager& tagalong_mngr,
        double dt);
}
}

#endif // !AnimationSystems_hpp
