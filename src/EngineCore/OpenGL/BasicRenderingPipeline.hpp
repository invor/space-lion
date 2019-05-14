#ifndef BasicRenderingPipeline
#define BasicRenderingPipeline

#include "../Frame.hpp"
#include "../WorldState.hpp"
#include "ResourceManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {
            void setupBasicForwardRenderingPipeline(
                Common::Frame&   frame,
                WorldState&      world_state,
                ResourceManager& resource_mngr);

            /** Experimenting with new Renderer architecture */
            void setupBasicRenderingPipeline(
                Common::Frame&   frame,
                WorldState&      world_state,
                ResourceManager& resource_mngr);
        }
    }
}

#endif // !BasicRenderingPipeline
