#ifndef SimpleForwardRenderingPipeline_hpp
#define SimpleForwardRenderingPipeline_hpp

namespace EngineCore
{
    namespace Common
    {
        struct Frame;
    }
	class WorldState;

	namespace Graphics
	{
		namespace Dx11
		{
            class ResourceManager;

			void setupSimpleForwardRenderingPipeline(Common::Frame& frame, WorldState& world, ResourceManager& resource_mngr);
		}
	}
}

#endif // !SimpleForwardRenderingPipeline_hpp
