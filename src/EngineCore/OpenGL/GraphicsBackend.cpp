#include "GraphicsBackend.hpp"

#include <iostream>

#include "../Frame.hpp"
#include "ResourceManager.hpp"

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>

#include <glad/glad.h>
#include <glfw3.h>

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {
            void GraphicsBackend::run(ResourceManager* resource_manager, Common::FrameManager* frame_manager)
            {
                // Initialize GLFW
                if (!glfwInit())
                {
                    std::cout << "-----\n"
                        << "The time is out of joint - O cursed spite,\n"
                        << "That ever I was born to set it right!\n"
                        << "-----\n"
                        << "Error: Couldn't initialize glfw.";
                }

#if EDITOR_MODE
                m_active_window = glfwCreateWindow(1600, 900, "Space-Lion", NULL, NULL);
                //m_active_window = glfwCreateWindow(1920, 1080, "Space-Lion", glfwGetPrimaryMonitor(), NULL);
#else
                //m_active_window = glfwCreateWindow(1920, 1080, "Space-Lion", glfwGetPrimaryMonitor(), NULL);
                m_active_window = glfwCreateWindow(1280, 720, "Space-Lion", NULL, NULL);
#endif

                if (!m_active_window)
                {
                    std::cout << "-----\n"
                        << "The time is out of joint - O cursed spite,\n"
                        << "That ever I was born to set it right!\n"
                        << "-----\n"
                        << "Error: Couldn't open glfw window";

                    glfwTerminate();
                }

                glfwMakeContextCurrent(m_active_window);

                // Get context version information
                int major = glfwGetWindowAttrib(m_active_window, GLFW_CONTEXT_VERSION_MAJOR);
                int minor = glfwGetWindowAttrib(m_active_window, GLFW_CONTEXT_VERSION_MINOR);

                std::cout << "OpenGL context " << major << "." << minor << std::endl;

                // Register callback functions
                glfwSetWindowSizeCallback(m_active_window, windowSizeCallback);
                glfwSetWindowCloseCallback(m_active_window, windowCloseCallback);

                glfwSetWindowUserPointer(m_active_window, this);

                // Init ImGui, set install_callbacks to false cause I call them myself
                //IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                if (!ImGui_ImplGlfw_InitForOpenGL(m_active_window, false))
                    std::cerr << "Error during imgui init " << std::endl;

                //Controls::setControlCallbacks(m_active_window);

                // Initialize glew
                //glewExperimental = GL_TRUE;
                if (!gladLoadGL()) {
                    std::cout << "-----\n"
                        << "The time is out of joint - O cursed spite,\n"
                        << "That ever I was born to set it right!\n"
                        << "-----\n"
                        << "Error during gladLoadGL.\n";
                    exit(-1);
                }
                std::cout << "OpenGL %d.%d\n" << GLVersion.major << " " << GLVersion.minor;

                assert((glGetError() == GL_NO_ERROR));

                double t0, t1 = 0.0;

                while (!glfwWindowShouldClose(m_active_window))
                {
                    t0 = t1;
                    t1 = glfwGetTime();
                    double dt = t1 - t0;

                    //std::cout<<"Timestep: "<<dt<<std::endl;

                    //Controls::checkKeyStatus(m_active_window, dt);
                    //Controls::checkJoystickStatus(m_active_window, dt);

                    // Perform single execution tasks
                    processSingleExecutionTasks();

                    // Perform resource manager async tasks
                    resource_manager->executeRenderThreadTasks();

                    // Get current frame for rendering
                    auto& frame = frame_manager->getRenderFrame();

                    // Call buffer phase for each render pass
                    for (auto& render_pass : frame.m_render_passes)
                    {
                        render_pass.setupResources();
                    }

                    auto gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR)
                        std::cerr << "GL error after resource setup of frame " << frame.m_frameID << " : " << gl_err << std::endl;

                    // Call execution phase for each render pass
                    for (auto& render_pass : frame.m_render_passes)
                    {
                        render_pass.execute();
                    }

                    gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR)
                        std::cerr << "GL error after execution of frame " << frame.m_frameID << " : " << gl_err << std::endl;

                    frame_manager->swapRenderFrame();

                    glfwSwapBuffers(m_active_window);
                    glfwPollEvents();
                }
            }

            void GraphicsBackend::addSingleExecutionGpuTask(std::function<void()> task)
            {
                m_singleExecution_tasks.push(task);
            }

            void GraphicsBackend::processSingleExecutionTasks()
            {
                auto start_time = std::chrono::steady_clock::now();

                size_t task_cnt = m_singleExecution_tasks.size();

                while (!m_singleExecution_tasks.empty())
                {
                    //auto t_0 = std::chrono::steady_clock::now();

                    std::function<void()> task = m_singleExecution_tasks.pop();
                    task();

                    glFinish();

                    auto t_1 = std::chrono::steady_clock::now();

                    //std::chrono::duration<double, std::milli> task_time = (t_1 - t_0);
                    //std::cout << "Task time: " << task_time.count() << std::endl;

                    std::chrono::duration<double, std::milli> time = (t_1 - start_time);

                    //if (time.count() > 33)
                    //	break;
                }
            }

            std::pair<int, int> GraphicsBackend::getActiveWindowResolution()
            {
                std::pair<int, int> retval;

                if (m_active_window != nullptr)
                {
                    glfwGetWindowSize(m_active_window, &std::get<0>(retval), &std::get<1>(retval));
                }

                return retval;
            }

            void GraphicsBackend::windowSizeCallback(GLFWwindow* window, int width, int height)
            {

            }

            void GraphicsBackend::windowCloseCallback(GLFWwindow* window)
            {

            }
        }
    }
}