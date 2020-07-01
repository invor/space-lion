#include "EngineFrontend.hpp"

#include <thread>
#include <vector>
#include <future>
#include <chrono>

#include "OpenGL/BasicRenderingPipeline.hpp"
#include "GeometryBakery.hpp"

#include "InputEvent.hpp"

namespace EngineCore
{
    namespace Common
    {
        EngineFrontend::EngineFrontend()
            : m_engine_started(false),
            m_task_schedueler(std::make_unique<Utility::TaskSchedueler>()),
            m_frame_manager(std::make_unique<FrameManager>()),
            m_graphics_backend(std::make_unique<Graphics::OpenGL::GraphicsBackend>()),
            m_resource_manager(std::make_unique<Graphics::OpenGL::ResourceManager>()),
            m_world_state(std::make_unique<WorldState>(m_resource_manager.get()))
        {
        }

        void EngineFrontend::startEngine()
        {
            // create initial frame
            size_t frameID = 0;

            // start rendering pipeline
            //std::thread render_thread(&(DeferredRenderingPipeline::run), &GEngineCore::renderingPipeline()));
            auto render_exec = std::async(std::launch::async, &(Graphics::OpenGL::GraphicsBackend::run), m_graphics_backend.get(), m_resource_manager.get(), m_frame_manager.get());
            auto render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

            //auto render_exec = std::async(std::launch::async, &(GraphicsBackend::run), &GEngineCore::graphicsBackend());
            //auto render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

            // start task schedueler with 1 thread
            m_task_schedueler->run(1);

            auto t_0 = std::chrono::high_resolution_clock::now();
            auto t_1 = std::chrono::high_resolution_clock::now();

            auto& entity_mngr = m_world_state->accessEntityManager();
            auto& camera_mngr = m_world_state->accessCameraComponentManager();
            auto& mtl_mngr = m_world_state->accessMaterialComponentManager();
            auto& mesh_mngr = m_world_state->accessMeshComponentManager();
            auto& rsrc_mngr = (*m_resource_manager);
            auto& renderTask_mngr = m_world_state->accessRenderTaskComponentManager();
            auto& transform_mngr = m_world_state->accessTransformManager();
            auto& turntable_mngr = m_world_state->accessTurntableManager();

            // inplace construct an input action context to test the new concept
            auto evt_func = [&camera_mngr,&transform_mngr](Input::Event const& evt, Input::HardwareState const& state) {
                std::cout << "Paying respect to new input system"<<"\n";
            };
            Input::EventDrivenAction evt_action = { {Input::Device::KEYBOARD,Input::KeyboardKeys::KEY_F,Input::EventTrigger::PRESS}, evt_func };
            Input::InputActionContext input_context = { "test_input_context", true, {evt_action}, {} };
            m_graphics_backend->addInputActionContext(input_context);

            // wait for window creation
            m_graphics_backend->waitForWindowCreation();

            // if everything is up and running, notify whoever is waiting for the engine to start up
            {
                std::lock_guard<std::mutex> lk(m_engine_started_mutex);
                m_engine_started = true;
            }
            m_engine_started_cVar.notify_one();

            // engine update loop
            while (render_exec_status != std::future_status::ready)
            {
                double dt = std::chrono::duration_cast<std::chrono::duration<double>>(t_1 - t_0).count();
                //std::cout << "dt: " << dt << std::endl;
                t_0 = std::chrono::high_resolution_clock::now();

                //TODO update world
                turntable_mngr.animate(dt);

                // finalize engine update by creating a new frame
                Frame new_frame;

                new_frame.m_frameID = frameID++;
                new_frame.m_dt = dt;

                auto window_res = m_graphics_backend->getActiveWindowResolution();

                //assert(std::get<0>(window_res) > 0);

                new_frame.m_window_width = std::get<0>(window_res);
                new_frame.m_window_height = std::get<1>(window_res);

                Entity camera_entity = camera_mngr.getActiveCamera();
                
                if (camera_entity != entity_mngr.invalidEntity())
                {
                    auto camera_idx = camera_mngr.getIndex(camera_entity).front();

                    size_t camera_transform_idx = transform_mngr.getIndex(camera_entity).front();
                    new_frame.m_view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx));
                    new_frame.m_projection_matrix = camera_mngr.getProjectionMatrix(camera_idx);
                    new_frame.m_fovy = camera_mngr.getFovy(camera_idx);
                    new_frame.m_aspect_ratio = camera_mngr.getAspectRatio(camera_idx);
                    new_frame.m_exposure = camera_mngr.getExposure(camera_idx);

                    Frame& update_frame = m_frame_manager->setUpdateFrame(std::move(new_frame));

                    Graphics::OpenGL::setupBasicForwardRenderingPipeline(update_frame, *m_world_state, *m_resource_manager);

                    m_frame_manager->swapUpdateFrame();
                }

                // check if rendering pipeline is still running
                render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

                t_1 = std::chrono::high_resolution_clock::now();
            }

            m_task_schedueler->stop();
        }

        void EngineFrontend::waitForEngineStarted()
        {
            std::unique_lock<std::mutex> lk(m_engine_started_mutex);
            m_engine_started_cVar.wait(lk, [this] {return m_engine_started; });
        }

        WorldState & EngineFrontend::accessWorldState()
        {
            return (*m_world_state.get());
        }

        FrameManager & EngineFrontend::accessFrameManager()
        {
            return (*m_frame_manager.get());
        }

        Graphics::OpenGL::ResourceManager& EngineFrontend::accessResourceManager()
        {
            return (*m_resource_manager.get());
        }

        void EngineFrontend::addInputActionContext(Input::InputActionContext const & input_action_context)
        {
            m_graphics_backend->addInputActionContext(input_action_context);
        }

    }
}