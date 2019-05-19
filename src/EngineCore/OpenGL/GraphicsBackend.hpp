#ifndef GraphicsBackend_hpp
#define GraphicsBackend_hpp

#include <functional>

#include "../MTQueue.hpp"

struct GLFWwindow;

namespace EngineCore
{
    namespace Common
    {
        class FrameManager;
    }

    namespace Graphics
    {
        namespace OpenGL
        {
            class ResourceManager;

            class GraphicsBackend
            {
            public:
                GraphicsBackend() : m_active_window(nullptr), m_singleExecution_tasks() {}
                ~GraphicsBackend() = default;

                /** Start and run graphics backend. Returns only after rendering window is closed. */
                void run(ResourceManager* resource_manager, Common::FrameManager* frame_manager);

                /** Add a task to the graphics backend that only has to be executed once */
                void addSingleExecutionGpuTask(std::function<void()> task);

                std::pair<int, int> getActiveWindowResolution();

            private:
                /** Pointer to active window */
                GLFWwindow* m_active_window;

                /** Thread-safe queue for tasks that have to be executed on the render thread, but only a single time */
                Utility::MTQueue<std::function<void()>> m_singleExecution_tasks;

                void processSingleExecutionTasks();

                /**************************************************************************
                * (Static) callbacks functions
                *************************************************************************/

                static void windowSizeCallback(GLFWwindow* window, int width, int height);
                static void windowCloseCallback(GLFWwindow* window);
            };
        }
    }
}

#endif // !GraphicsBackend
