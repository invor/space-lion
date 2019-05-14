#ifndef EngineFrontend_hpp
#define EngineFrontend_hpp

#include "Frame.hpp"

#include "OpenGL/GraphicsBackend.hpp"
#include "OpenGL/ResourceManager.hpp"
#include "TaskSchedueler.hpp"
#include "WorldState.hpp"

namespace EngineCore
{
	namespace Common
	{
		class EngineFrontend
		{
		public:
			EngineFrontend();
			~EngineFrontend() = default;

			void startEngine();

		private:
			void createDemoScene();

			/**
			 * Simple multi-thread task schedueler.
			 */
			std::unique_ptr<Utility::TaskSchedueler> m_task_schedueler;

			/**
			 * The frame manager used to carry over snapshots of the world simulation to rendering.
			 */
			std::unique_ptr<FrameManager> m_frame_manager;

			/**
			 * Graphics backend used for rendering.
			 */
			std::unique_ptr<Graphics::OpenGL::GraphicsBackend> m_graphics_backend;

			/** 
			 * GPU resource manager.
			 */
			std::unique_ptr<Graphics::OpenGL::ResourceManager> m_resource_manager;

			/**
			 * Collection of all component manager that make up the world (state).
			 */
			std::unique_ptr<WorldState> m_world_state;

		};
	}
}

#endif // !EngineFrontend_hpp
