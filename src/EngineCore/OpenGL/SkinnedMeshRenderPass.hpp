#ifndef SkinnedMeshRenderPass_hpp
#define SkinnedMeshRenderPass_hpp

#include "../Frame.hpp"
#include "../WorldState.hpp"
#include "ResourceManager.hpp"

namespace EngineCore {
    namespace Graphics {
        namespace OpenGL {

            void addSkinnedMeshRenderPass(
                Common::Frame& frame,
                WorldState& world_state,
                ResourceManager& resource_mngr);

        }
    }
}

#endif // !SkinnedMeshRenderPass_hpp
