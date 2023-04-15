#ifndef OpenGL_LandscapeSystems_hpp
#define OpenGL_LandscapeSystems_hpp

#include "GraphicsBackend.hpp"
#include "LandscapeBrickComponent.hpp"
#include "ResourceManager.hpp"

namespace EngineCore {
namespace Graphics {
namespace Landscape {
namespace OpenGL {

    void updateBricks(
        Graphics::OpenGL::GraphicsBackend& graphics_backend,
        Graphics::OpenGL::ResourceManager& resource_mngr,
        LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>& lscp_brick_mngr,
        std::vector<LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent>& landscape_bricks,
        std::vector<Entity> const& brick_entities);

}
}
}
}

#endif