#ifndef GraphicsBackend_hpp
#define GraphicsBackend_hpp

#include <condition_variable>
#include <functional>

#include "Frame.hpp"
#include "MTQueue.hpp"
#include "InputEvent.hpp"

struct GLFWwindow;

namespace EngineCore
{
    //namespace Common
    //{
    //    struct Frame;
    //    template<>
    //    class FrameManager<typename Frame>;
    //}

    namespace Graphics
    {
        namespace OpenGL
        {
            class ResourceManager;

            class GraphicsBackend
            {
            public:
                GraphicsBackend() : m_active_window(nullptr), m_singleExecution_tasks(), m_window_created(false) {}
                ~GraphicsBackend() = default;

                /** Start and run graphics backend. Returns only after rendering window is closed. */
                void run(ResourceManager* resource_manager, Common::FrameManager<Common::Frame>* frame_manager);

                /** Add a task to the graphics backend that only has to be executed once */
                void addSingleExecutionGpuTask(std::function<void()> task);

                std::pair<int, int> getActiveWindowResolution();

                /** Function blocks until window is created */
                void waitForWindowCreation();

                void addInputActionContext(Common::Input::InputActionContext const& context);

                void removeInputActionContext(std::string const& context_name);

                void setInputActionContextActive(std::string const& context_name);

                void setInputActionContextInactive(std::string const& context_name);

            private:
                /** Pointer to active window */
                GLFWwindow* m_active_window;

                /** Thread-safe queue for tasks that have to be executed on the render thread, but only a single time */
                Utility::MTQueue<std::function<void()>> m_singleExecution_tasks;

                bool m_window_created;
                std::mutex m_window_creation_mutex;
                std::condition_variable m_winodw_creation_cVar;

                // List of input contexts
                std::list<Common::Input::InputActionContext> m_input_action_contexts;

                void processSingleExecutionTasks();

                /**************************************************************************
                * (Static) callbacks functions
                *************************************************************************/

                static void windowSizeCallback(GLFWwindow* window, int width, int height);
                static void windowCloseCallback(GLFWwindow* window);

                static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
                static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
                static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
                static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            };
        }
    }
}

#endif // !GraphicsBackend
