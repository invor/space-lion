#include "AtmosphereRenderPass.hpp"

void EngineCore::Graphics::OpenGL::addAtmosphereRenderPass(Common::Frame& frame, WorldState& world_state, ResourceManager& resource_mngr)
{
    struct AtmospherePassData
    {
    };

    struct AtmospherePassResources
    {
    };

    // Compositing pass
    frame.addRenderPass<AtmospherePassData, AtmospherePassResources>("AtmospherePass",
        // data setup phase
        [&world_state, &resource_mngr](AtmospherePassData& data, AtmospherePassResources& resources) {

        },
        // resource setup phase
        [&world_state, &resource_mngr](AtmospherePassData& data, AtmospherePassResources& resources) {

        },
        // execute phase
        [&frame](AtmospherePassData const& data, AtmospherePassResources const& resources) {
            
        }
    );
}
