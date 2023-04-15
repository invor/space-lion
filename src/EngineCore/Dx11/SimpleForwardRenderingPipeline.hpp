#ifndef SimpleForwardRenderingPipeline_hpp
#define SimpleForwardRenderingPipeline_hpp

namespace EngineCore
{
	class WorldState;

	namespace Graphics
	{
		namespace Dx11
		{
			struct Frame;
            class ResourceManager;

			void setupSimpleForwardRenderingPipeline(Frame & frame, WorldState& world, ResourceManager& resource_mngr);
		}
	}
}

#endif // !SimpleForwardRenderingPipeline_hpp
