#ifndef OceanRenderPass_hpp
#define OceanRenderPass_hpp

#include "../Frame.hpp"
#include "../WorldState.hpp"
#include "ResourceManager.hpp"

namespace EngineCore {
namespace Graphics {
namespace OpenGL {

    void addOceanRenderPass(Common::Frame& frame,
        WorldState& world_state,
        ResourceManager& resource_mngr);

}
}
}

#endif // !OceanRenderPass_hpp
