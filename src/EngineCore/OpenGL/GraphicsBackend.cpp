#include "GraphicsBackend.hpp"

#include <iostream>

#include "ResourceManager.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {
            namespace {
                /*
                * BSD 3-Clause License
                * 
                * Copyright (c) 2007-2021, MegaMol Dev Team
                * Copyright (c) 2007-2021, Visualization Research Center (VISUS), University of Stuttgart
                * All rights reserved.
                * 
                * Redistribution and use in source and binary forms, with or without
                * modification, are permitted provided that the following conditions are met:
                * 
                * 1. Redistributions of source code must retain the above copyright notice, this
                *    list of conditions and the following disclaimer.
                * 
                * 2. Redistributions in binary form must reproduce the above copyright notice,
                *    this list of conditions and the following disclaimer in the documentation
                *    and/or other materials provided with the distribution.
                * 
                * 3. Neither the name of the copyright holder nor the names of its
                *    contributors may be used to endorse or promote products derived from
                *    this software without specific prior written permission.
                * 
                * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
                * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
                * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
                * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
                * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
                * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
                * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
                * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
                * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
                * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
                * 
                */
#ifdef _WIN32
                static std::string GetStack() {
                    unsigned int i;
                    void* stack[100];
                    unsigned short frames;
                    SYMBOL_INFO* symbol;
                    HANDLE process;
                    std::stringstream output;

                    process = GetCurrentProcess();

                    SymSetOptions(SYMOPT_LOAD_LINES);

                    SymInitialize(process, NULL, TRUE);

                    frames = CaptureStackBackTrace(0, 200, stack, NULL);
                    symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
                    symbol->MaxNameLen = 255;
                    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

                    for (i = 0; i < frames; i++) {
                        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
                        DWORD dwDisplacement;
                        IMAGEHLP_LINE64 line;

                        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                        if (!strstr(symbol->Name, "khr::getStack") && !strstr(symbol->Name, "khr::DebugCallback") &&
                            SymGetLineFromAddr64(process, (DWORD64)(stack[i]), &dwDisplacement, &line)) {

                            output << "function: " << symbol->Name << " - line: " << line.LineNumber << "\n";
                        }
                        if (0 == strcmp(symbol->Name, "main"))
                            break;
                    }

                    free(symbol);
                    return output.str();
                }
#endif
                static std::string get_message_id_name(GLuint id) {
                    if (id == 0x0500) {
                        return "GL_INVALID_ENUM";
                    }
                    if (id == 0x0501) {
                        return "GL_INVALID_VALUE";
                    }
                    if (id == 0x0502) {
                        return "GL_INVALID_OPERATION";
                    }
                    if (id == 0x0503) {
                        return "GL_STACK_OVERFLOW";
                    }
                    if (id == 0x0504) {
                        return "GL_STACK_UNDERFLOW";
                    }
                    if (id == 0x0505) {
                        return "GL_OUT_OF_MEMORY";
                    }
                    if (id == 0x0506) {
                        return "GL_INVALID_FRAMEBUFFER_OPERATION";
                    }
                    if (id == 0x0507) {
                        return "GL_CONTEXT_LOST";
                    }
                    if (id == 0x8031) {
                        return "GL_TABLE_TOO_LARGE";
                    }

                    return std::to_string(id);
                }

                static void APIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar* message, const void* userParam) {
                    /* Message Sources
                        Source enum                      Generated by
                        GL_DEBUG_SOURCE_API              Calls to the OpenGL API
                        GL_DEBUG_SOURCE_WINDOW_SYSTEM    Calls to a window - system API
                        GL_DEBUG_SOURCE_SHADER_COMPILER  A compiler for a shading language
                        GL_DEBUG_SOURCE_THIRD_PARTY      An application associated with OpenGL
                        GL_DEBUG_SOURCE_APPLICATION      Generated by the user of this application
                        GL_DEBUG_SOURCE_OTHER            Some source that isn't one of these
                    */
                    /* Message Types
                        Type enum                          Meaning
                        GL_DEBUG_TYPE_ERROR                An error, typically from the API
                        GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR  Some behavior marked deprecated has been used
                        GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR   Something has invoked undefined behavior
                        GL_DEBUG_TYPE_PORTABILITY          Some functionality the user relies upon is not portable
                        GL_DEBUG_TYPE_PERFORMANCE          Code has triggered possible performance issues
                        GL_DEBUG_TYPE_MARKER               Command stream annotation
                        GL_DEBUG_TYPE_PUSH_GROUP           Group pushing
                        GL_DEBUG_TYPE_POP_GROUP            foo
                        GL_DEBUG_TYPE_OTHER                Some type that isn't one of these
                    */
                    /* Message Severity
                        Severity enum                    Meaning
                        GL_DEBUG_SEVERITY_HIGH           All OpenGL Errors, shader compilation / linking errors, or highly
                                                         - dangerous undefined behavior
                        GL_DEBUG_SEVERITY_MEDIUM         Major performance warnings, shader compilation / linking
                                                         warnings, or the use of deprecated functionality
                        GL_DEBUG_SEVERITY_LOW            Redundant state change
                                                         performance warning, or unimportant undefined behavior
                        GL_DEBUG_SEVERITY_NOTIFICATION   Anything that isn't an
                                                         error or performance issue.
                    */
                    // if (source == GL_DEBUG_SOURCE_API || source == GL_DEBUG_SOURCE_SHADER_COMPILER)
                    //    if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ||
                    //        type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
                    //        if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
                    //            std::cout << "OpenGL Error: " << message << " (" << get_message_id_name(id) << ")" << std::endl;

                    std::string sourceText, typeText, severityText;
                    switch (source) {
                    case GL_DEBUG_SOURCE_API:
                        sourceText = "API";
                        break;
                    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                        sourceText = "Window System";
                        break;
                    case GL_DEBUG_SOURCE_SHADER_COMPILER:
                        sourceText = "Shader Compiler";
                        break;
                    case GL_DEBUG_SOURCE_THIRD_PARTY:
                        sourceText = "Third Party";
                        break;
                    case GL_DEBUG_SOURCE_APPLICATION:
                        sourceText = "Application";
                        break;
                    case GL_DEBUG_SOURCE_OTHER:
                        sourceText = "Other";
                        break;
                    default:
                        sourceText = "Unknown";
                        break;
                    }
                    switch (type) {
                    case GL_DEBUG_TYPE_ERROR:
                        typeText = "Error";
                        break;
                    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                        typeText = "Deprecated Behavior";
                        break;
                    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                        typeText = "Undefined Behavior";
                        break;
                    case GL_DEBUG_TYPE_PORTABILITY:
                        typeText = "Portability";
                        break;
                    case GL_DEBUG_TYPE_PERFORMANCE:
                        typeText = "Performance";
                        break;
                    case GL_DEBUG_TYPE_MARKER:
                        typeText = "Marker";
                        break;
                    case GL_DEBUG_TYPE_PUSH_GROUP:
                        typeText = "Push Group";
                        break;
                    case GL_DEBUG_TYPE_POP_GROUP:
                        typeText = "Pop Group";
                        break;
                    case GL_DEBUG_TYPE_OTHER:
                        typeText = "Other";
                        break;
                    default:
                        typeText = "Unknown";
                        break;
                    }
                    switch (severity) {
                    case GL_DEBUG_SEVERITY_HIGH:
                        severityText = "High";
                        break;
                    case GL_DEBUG_SEVERITY_MEDIUM:
                        severityText = "Medium";
                        break;
                    case GL_DEBUG_SEVERITY_LOW:
                        severityText = "Low";
                        break;
                    case GL_DEBUG_SEVERITY_NOTIFICATION:
                        severityText = "Notification";
                        break;
                    default:
                        severityText = "Unknown";
                        break;
                    }

                    std::stringstream output;
                    output << "[" << sourceText << " " << severityText << "] (" << typeText << " " << id << " ["
                        << get_message_id_name(id) << "]) " << message << std::endl
                        << "stack trace:" << std::endl;
#ifdef _WIN32
                    output << GetStack() << std::endl;
                    OutputDebugStringA(output.str().c_str());
#endif

                    if (type == GL_DEBUG_TYPE_ERROR) {
                        std::cerr << output.str();
                    }
                    //else if (type == GL_DEBUG_TYPE_OTHER || type == GL_DEBUG_TYPE_MARKER) {
                    //}
                    else {
                        std::cout << output.str();
                    }
                }
            }

            void GraphicsBackend::run(ResourceManager* resource_manager, Common::FrameManager<Common::Frame>* frame_manager)
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
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
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

                //glfwSwapInterval(1);

                // Get context version information
                int major = glfwGetWindowAttrib(m_active_window, GLFW_CONTEXT_VERSION_MAJOR);
                int minor = glfwGetWindowAttrib(m_active_window, GLFW_CONTEXT_VERSION_MINOR);

                std::cout << "OpenGL context " << major << "." << minor << std::endl;

                // Register callback functions
                glfwSetWindowSizeCallback(m_active_window, windowSizeCallback);
                glfwSetWindowCloseCallback(m_active_window, windowCloseCallback);
                glfwSetKeyCallback(m_active_window, keyCallback);

                glfwSetWindowUserPointer(m_active_window, this);

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

                //glDebugMessageCallback(opengl_debug_message_callback, NULL);

                {
                    std::lock_guard<std::mutex> lk(m_window_creation_mutex);
                    m_window_created = true;
                }
                m_winodw_creation_cVar.notify_one();

                // Init ImGui, set install_callbacks to false cause I call them myself
                //IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                if (!ImGui_ImplGlfw_InitForOpenGL(m_active_window, false))
                    std::cerr << "Error during imgui init " << std::endl;
                ImGui_ImplOpenGL3_Init("#version 450");
                //Controls::setControlCallbacks(m_active_window);

                size_t render_frameID = 0;
                double t0, t1 = 0.0;

                while (!glfwWindowShouldClose(m_active_window))
                {
                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    // Perform single execution tasks
                    processSingleExecutionTasks();

                    auto gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR)
                        std::cerr << "GL error after single exection tasks: " << gl_err << std::endl;

                    // Perform resource manager async tasks
                    resource_manager->executeRenderThreadTasks();

                    gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR)
                        std::cerr << "GL error after resource manager tasks: " << gl_err << std::endl;

                    // TODO try getting update for render frame ?

                    // Get current frame for rendering
                    auto& frame = frame_manager->getRenderFrame();
                    frame.m_render_frameID = render_frameID++;

                    t0 = t1;
                    t1 = glfwGetTime();
                    double dt = t1 - t0;

                    // TODO dedicated dt for input computations...
                    frame.m_render_dt = dt;

                    //std::cout<<"Timestep: "<<dt<<std::endl;

                    for (auto& input_context : m_input_action_contexts)
                    {
                        if (input_context.m_is_active)
                        {
                            for (auto& state_action : input_context.m_state_actions)
                            {
                                std::vector<Common::Input::HardwareState> states;

                                for (auto& part : state_action.m_state_query)
                                {
                                    if (std::get<0>(part) == Common::Input::Device::KEYBOARD)
                                    {
                                        states.emplace_back( glfwGetKey(m_active_window, std::get<1>(part)) == GLFW_PRESS ? 1.0f : 0.0 );
                                    }
                                    else if (std::get<0>(part) == Common::Input::Device::MOUSE_AXES)
                                    {
                                        double x, y;
                                        glfwGetCursorPos(m_active_window, &x, &y);

                                        if (std::get<1>(part) == Common::Input::MouseAxes::MOUSE_CURSOR_X)
                                        {
                                            states.emplace_back(x);
                                        }
                                        else if (std::get<1>(part) == Common::Input::MouseAxes::MOUSE_CURSOR_Y)
                                        {
                                            states.emplace_back(y);
                                        }
                                    }
                                    else if (std::get<0>(part) == Common::Input::Device::MOUSE_BUTTON)
                                    {
                                        if (std::get<1>(part) == Common::Input::MouseButtons::MOUSE_BUTTON_RIGHT)
                                        {
                                            states.emplace_back( glfwGetMouseButton(m_active_window, std::get<1>(part)) == GLFW_PRESS ? 1.0f : 0.0f );
                                        }
                                    }
                                }

                                state_action.m_action(state_action.m_state_query, states, dt);

                            }
                        }
                    }


                    // Call buffer phase for each render pass
                    for (auto& render_pass : frame.m_render_passes)
                    {
                        render_pass.setupResources();
                    }

                    gl_err = glGetError();
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


                    int width, height;
                    glfwGetFramebufferSize(m_active_window, &width, &height);
                    
                    
                    ImGui::SetNextWindowPos(ImVec2(width - 375.0f, height - 100.0f));
                    bool p_open = true;
                    if (!ImGui::Begin("FPS", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
                    {
                        ImGui::End();
                        return;
                    }
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::End();
                    ImGui::Render();
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                    frame_manager->swapRenderFrame();

                    glfwSwapBuffers(m_active_window);
                    glfwPollEvents();
                }

                resource_manager->clearAllResources();
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
                else
                {
                    std::cerr << "Trying to get window resolution from nullptr." << std::endl;
                }

                return retval;
            }

            void GraphicsBackend::waitForWindowCreation()
            {
                std::unique_lock<std::mutex> lk(m_window_creation_mutex);
                m_winodw_creation_cVar.wait(lk, [this] {return m_window_created; });
            }

            void GraphicsBackend::addInputActionContext(Common::Input::InputActionContext const& context)
            {
                m_input_action_contexts.push_back(context);
            }

            void GraphicsBackend::windowSizeCallback(GLFWwindow* window, int width, int height)
            {

            }

            void GraphicsBackend::windowCloseCallback(GLFWwindow* window)
            {

            }

            void GraphicsBackend::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
            {
            }

            void GraphicsBackend::mouseCursorCallback(GLFWwindow * window, double xpos, double ypos)
            {
            }

            void GraphicsBackend::mouseScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
            {
            }

            void GraphicsBackend::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
            {
                auto graphics_backend = reinterpret_cast<GraphicsBackend*>(glfwGetWindowUserPointer(window));

                for (auto& input_context : graphics_backend->m_input_action_contexts)
                {
                    if (input_context.m_is_active)
                    {
                        for (auto& event_action : input_context.m_event_actions)
                        {
                            if (std::get<0>(event_action.m_event) == Common::Input::Device::KEYBOARD &&
                                std::get<1>(event_action.m_event) == key &&
                                std::get<2>(event_action.m_event) == action
                                )
                            {
                                event_action.m_action(event_action.m_event,/*TODO map action to meanigful state?*/1.0);
                            }
                        }
                    }
                }
            }
        }
    }
}