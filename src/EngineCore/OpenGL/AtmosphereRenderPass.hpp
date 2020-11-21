#ifndef AtmosphereRenderPass_hpp
#define AtmosphereRenderPass_hpp

#include "../Frame.hpp"
#include "../WorldState.hpp"
#include "ResourceManager.hpp"

namespace EngineCore{
namespace Graphics{
namespace OpenGL{

    void addAtmosphereRenderPass(
        Common::Frame& frame,
        WorldState& world_state,
        ResourceManager& resource_mngr);

}
}
}

#endif // !AtmosphereRenderPass_hpp
