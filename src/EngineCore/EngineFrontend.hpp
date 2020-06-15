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

            void waitForEngineStarted();

            /**
             * Grant access to world for the "driving" part of the application, e.g. editor, game, simulation etc.
             */
            WorldState& accessWorldState();

            /**
             * Grant access to frame data for the "driving" part of the application, e.g. editor, game, simulation etc.
             */
            FrameManager& accessFrameManager();

            /**
             * Pass on an input action context to window handling in graphics backend
             */
            void addInputActionContext(InputActionContext const& input_action_context);

        private:
            void createDemoScene();

            bool m_engine_started;
            std::mutex m_engine_started_mutex;
            std::condition_variable m_engine_started_cVar;

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
