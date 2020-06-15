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
            : m_task_schedueler(std::make_unique<Utility::TaskSchedueler>()),
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

            createDemoScene();

            auto& entity_mngr = m_world_state->accessEntityManager();
            auto& camera_mngr = m_world_state->accessCameraComponentManager();
            auto& mtl_mngr = m_world_state->accessMaterialComponentManager();
            auto& mesh_mngr = m_world_state->accessMeshComponentManager();
            auto& rsrc_mngr = (*m_resource_manager);
            auto& renderTask_mngr = m_world_state->accessRenderTaskComponentManager();
            auto& transform_mngr = m_world_state->accessTransformManager();
            auto& turntable_mngr = m_world_state->accessTurntableManager();

            // inplace construct an input action context to test the new concept

            auto evt_func = [&camera_mngr,&transform_mngr](InputEvent const& evt) {
                std::cout << "Paying respect to new input system"<<"\n";
            };

            auto cam_ctrl_func = [this,&camera_mngr, &transform_mngr](InputState const& state) {

                uint camera_idx = camera_mngr.getActiveCameraIndex();
                Entity camera_entity = camera_mngr.getEntity(camera_idx);
                size_t camera_transform_idx = transform_mngr.getIndex(camera_entity).front();

                // first hardware part is w key, state value greater 0 shows that key is currently pressed
                if (std::get<2>(state[0]) > 0.0f)
                {
                    auto dt = m_frame_manager->getRenderFrame().m_dt;
                    transform_mngr.translate(camera_transform_idx, Vec3(1.0f * dt, 0.0f, 0.0f));
                }
            };

            InputEventAction evt_action = { {Input::Device::KEYBOARD,Input::KeyboardKeys::KEY_F,Input::EventTrigger::PRESS, 1.0f}, evt_func };
            InputStateAction state_action = { {{Input::Device::KEYBOARD,Input::KeyboardKeys::KEY_W, 1.0f}}, cam_ctrl_func };
            InputActionContext input_context = { "test_input_context", true, {evt_action}, {} };
            m_graphics_backend->addInputActionContext(input_context);

            // wait for window creation
            m_graphics_backend->waitForWindowCreation();

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

                uint camera_idx = camera_mngr.getActiveCameraIndex();
                Entity camera_entity = camera_mngr.getEntity(camera_idx);
                size_t camera_transform_idx = transform_mngr.getIndex(camera_entity).front();
                new_frame.m_view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx));
                new_frame.m_projection_matrix = camera_mngr.getProjectionMatrix(camera_idx);
                new_frame.m_fovy = camera_mngr.getFovy(camera_idx);
                new_frame.m_aspect_ratio = camera_mngr.getAspectRatio(camera_idx);
                new_frame.m_exposure = camera_mngr.getExposure(camera_idx);

                Frame& update_frame = m_frame_manager->setUpdateFrame(std::move(new_frame));

                Graphics::OpenGL::setupBasicForwardRenderingPipeline(update_frame, *m_world_state, *m_resource_manager);

                m_frame_manager->swapUpdateFrame();

                // check if rendering pipeline is still running
                render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

                t_1 = std::chrono::high_resolution_clock::now();
            }

            m_task_schedueler->stop();
        }

        void EngineFrontend::createDemoScene()
        {
            auto& entity_mngr = m_world_state->accessEntityManager();
            auto& camera_mngr = m_world_state->accessCameraComponentManager();
            auto& gltf_mngr = m_world_state->accessGltfAssetComponentManager();
            auto& mtl_mngr = m_world_state->accessMaterialComponentManager();
            auto& mesh_mngr = m_world_state->accessMeshComponentManager();
            auto& rsrc_mngr = (*m_resource_manager);
            auto& renderTask_mngr = m_world_state->accessRenderTaskComponentManager();
            auto& transform_mngr = m_world_state->accessTransformManager();
            auto& turntable_mngr = m_world_state->accessTurntableManager();

            auto camera = entity_mngr.create();
            transform_mngr.addComponent(camera,Vec3(0.0,0.0,2.5));
            camera_mngr.addComponent(camera);

            //TODO create turntable animation
            auto cube = entity_mngr.create();
            transform_mngr.addComponent(cube, Vec3(0.0, 0.0, -5.0));
            turntable_mngr.addComponent(cube, 0.5f);
            auto mesh_data = Graphics::createBox();
            auto mesh_rsrc = mesh_mngr.addComponent(
                cube,
                "demo_plane",
                std::get<0>(mesh_data),
                std::get<1>(mesh_data),
                std::make_shared<glowl::VertexLayout>(rsrc_mngr.convertGenericGltfVertexLayout(*std::get<2>(mesh_data))),
                GL_UNSIGNED_INT,
                GL_TRIANGLES);

            auto shader_names = std::make_shared<std::vector<Graphics::OpenGL::ResourceManager::ShaderFilename>>(
                std::initializer_list<Graphics::OpenGL::ResourceManager::ShaderFilename>{
                    { "../resources/shaders/simple_forward_vert.glsl", glowl::GLSLProgram::VertexShader },
                    { "../resources/shaders/fwd_pbrMetallic_f.glsl", glowl::GLSLProgram::FragmentShader }
            });

            auto shader_rsrc = rsrc_mngr.createShaderProgramAsync(
                "debug_forward_shader",
                shader_names
            );

            mtl_mngr.addComponent(cube, "debug_cube_material", shader_rsrc);

            //renderTask_mngr.addComponent(cube, mesh_rsrc, 0, shader_rsrc, 0);

            //Graphics::OpenGL::loadGLTFScene("../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", *m_world_state, rsrc_mngr);

              auto gltf_root = entity_mngr.create();
              transform_mngr.addComponent(gltf_root, Vec3(0.0, 0.0, 0.0));
              turntable_mngr.addComponent(gltf_root, 0.5f);
              
              {
                  auto gltf_subobj = entity_mngr.create();
                  size_t transform_idx = transform_mngr.addComponent(gltf_subobj, Vec3(0.0, 0.0, 0.0));
                  transform_mngr.setParent(transform_idx, gltf_root);
                  gltf_mngr.addComponent(gltf_subobj, "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", "Hose_low", shader_rsrc);
              }

              {
                  auto gltf_subobj = entity_mngr.create();
                  size_t transform_idx = transform_mngr.addComponent(gltf_subobj, Vec3(0.0, 0.0, 0.0));
                  transform_mngr.setParent(transform_idx, gltf_root);
                  gltf_mngr.addComponent(gltf_subobj, "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", "RubberWood_low", shader_rsrc);
              }

              {
                  auto gltf_subobj = entity_mngr.create();
                  size_t transform_idx = transform_mngr.addComponent(gltf_subobj, Vec3(0.0, 0.0, 0.0));
                  transform_mngr.setParent(transform_idx, gltf_root);
                  gltf_mngr.addComponent(gltf_subobj, "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", "GlassPlastic_low", shader_rsrc);
              }
              
              {
                  auto gltf_subobj = entity_mngr.create();
                  size_t transform_idx = transform_mngr.addComponent(gltf_subobj, Vec3(0.0, 0.0, 0.0));
                  transform_mngr.setParent(transform_idx, gltf_root);
                  gltf_mngr.addComponent(gltf_subobj, "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", "MetalParts_low", shader_rsrc);
              }
              
              {
                  auto gltf_subobj = entity_mngr.create();
                  size_t transform_idx = transform_mngr.addComponent(gltf_subobj, Vec3(0.0, 0.0, 0.0));
                  transform_mngr.setParent(transform_idx, gltf_root);
                  gltf_mngr.addComponent(gltf_subobj, "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", "LeatherParts_low", shader_rsrc);
              }

              {
                  auto gltf_subobj = entity_mngr.create();
                  size_t transform_idx = transform_mngr.addComponent(gltf_subobj, Vec3(0.0, 0.0, 0.0));
                  transform_mngr.setParent(transform_idx, gltf_root);
                  gltf_mngr.addComponent(gltf_subobj, "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", "Lenses_low", shader_rsrc);
              }

            //gltf_mngr.importGltfScene("../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf", shader_rsrc);


            /*Entity debug_entity = m_entity_manager.create();

            m_transform_manager.addComponent(debug_entity,Vec3(0.0f,0.0f,-2.0f),Quat::CreateFromAxisAngle(Vec3(0.0f,1.0f,0.0f),0.0f), Vec3(1.0f, 1.0f, 1.0f));

            auto triangle_vertices = std::make_shared<std::vector<std::vector<float>>>();
            (*triangle_vertices) = { {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
                                        {-0.25f, 0.0f, 0.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f},
                                        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f} };

            auto triangle_indices = std::make_shared<std::vector<uint32_t>>(3);
            (*triangle_indices) = { 0,1,2 };

            auto vertex_layout = std::make_shared<VertexLayout>();
            vertex_layout->strides = { 12, 12, 8 };
            vertex_layout->attributes = {
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            auto mesh_rsrc = m_mesh_manager.addComponent(debug_entity, "debug_mesh", triangle_vertices, triangle_indices, vertex_layout, DXGI_FORMAT_R32_UINT, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            typedef std::pair < std::wstring, Graphics::Dx11::ShaderProgram::ShaderType > ShaderFilename;

            auto shader_names = std::make_shared<std::vector<ShaderFilename>>(
                std::initializer_list<ShaderFilename>{
                    { L"ms-appx:///gltfVertexShader.cso", Graphics::Dx11::ShaderProgram::VertexShader },
                    { L"ms-appx:///GeometryShader.cso", Graphics::Dx11::ShaderProgram::GeometryShader },
                    { L"ms-appx:///PixelShader.cso", Graphics::Dx11::ShaderProgram::PixelShader }
                }
            );

            auto shader_rsrc = m_resource_manager.createShaderProgramAsync(
                "triangle_debug_shader",
                shader_names,
                vertex_layout);

            m_material_manager.addComponent(debug_entity, "debug_material", shader_rsrc);

            m_render_task_manager.addComponent(debug_entity, mesh_rsrc, 0, shader_rsrc, 0);*/


            //		// Create test scene
            //		Entity camera_entity = GEngineCore::entityManager().create();
            //		GCoreComponents::transformManager().addComponent(camera_entity, Vec3(0.0, 1.0, -20.0), Quat(), Vec3(1.0));
            //		GCoreComponents::cameraManager().addComponent(camera_entity);
            //		GCoreComponents::cameraManager().setActiveCamera(camera_entity);
            //		GTools::selectManager().addComponent(camera_entity);
            //
            //		Entity sun_entity = GEngineCore::entityManager().create();
            //		//GCoreComponents::transformManager().addComponent(sun_entity, Vec3(149600000000.0f, 84960000000.0f, 84960000000.0f), Quat(), Vec3(1.0));
            //		//GCoreComponents::transformManager().addComponent(sun_entity, Vec3(0.0, 149600000000.0, 0.0), Quat(), Vec3(1.0));
            //		GCoreComponents::transformManager().addComponent(sun_entity, Vec3(105783174465.5f, 105783174465.5f, 5783174465.5f), Quat(), Vec3(1.0f));
            //		GCoreComponents::sunlightManager().addComponent(sun_entity, Vec3(1.0f), 3.75f *std::pow(10.0f, 28.0f), 696342000.0f);
            //		GTools::selectManager().addComponent(sun_entity);
            //
            //		//fill light
            //		//Entity light = GEngineCore::entityManager().create();
            //		//GCoreComponents::transformManager().addComponent(light, Vec3(512.0, 32.0, 762.0));
            //		//GCoreComponents::pointlightManager().addComponent(light, Vec3(1.0), 5000000000, 1000);
            //		//GRenderingComponents::interfaceMeshManager().addComponent(light, "../resources/materials/editor/interface_pointlight.slmtl", "../resources/meshes/editor/pointlight.fbx");
            //		//GRenderingComponents::pickingManager().addComponent(light, "../resources/materials/editor/picking.slmtl", "../resources/meshes/editor/pointlight.fbx");
            //		//GTools::selectManager().addComponent(light, [light]() { GTools::transformTool().activate(); }, [light]() { GTools::transformTool().deactivate(); });
            //
            //		//Entity sun_entity_2 = entity_mngr.create();
            //		//scene_transformations_mngr.addComponent(sun_entity_2,Vec3(149600000000.0,149600000.0,0.0),Quat(),Vec3(1.0));
            //		//sunlight_mngr.addComponent(sun_entity_2,Vec3(1.0), 3.75f *std::pow(10.0,28.0), 150000000000.0 );
            //
            //		//	for(int j=-10; j < 10; j = j+1)
            //		//	{
            //		//		Entity light_entity = entity_mngr.create();
            //		//		scene_transformations_mngr.addComponent(light_entity,Vec3(0.0,(float)j, (float)j ),Quat(),Vec3(1.0));
            //		//		light_mngr.addComponent(light_entity,Vec3(1.0),1200.0,1000.0);
            //		//	}
            //
            //		Entity atmoshphere_entity = GEngineCore::entityManager().create();
            //		//GCoreComponents::transformManager().addComponent(atmoshphere_entity, Vec3(0.0, -6360000.0, 0.0), Quat(), Vec3(6800000.0)); // check the correct size for the earth's atmosphere
            //		GCoreComponents::transformManager().addComponent(atmoshphere_entity, Vec3(0.0, -6360050.0, 0.0), Quat(), Vec3(6800000.0)); // check the correct size for the earth's atmosphere
            //		GRenderingComponents::atmosphereManager().addComponent(atmoshphere_entity,
            //			//Vec3(0.0058,0.0135,0.0331),
            //			//Vec3(0.00444,0.00444,0.00444),
            //			Vec3(0.0000058, 0.0000135, 0.0000331),
            //			//Vec3(0.00000444, 0.00000444, 0.00000444),
            //			Vec3(0.000003, 0.000003, 0.000003),
            //			//Vec3(0.00002,0.00002,0.00002),
            //			8000.0,
            //			1200.0,
            //			6360000.0,
            //			//6359800.0,
            //			6420000.0);
            //		//GTools::selectManager().addComponent(atmoshphere_entity);
            //
            //		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([]() {
            //		//	
            //		//	Entity decal_entity_4 = GEngineCore::entityManager().create();
            //		//	GCoreComponents::transformManager().addComponent(decal_entity_4, Vec3(0.0f, 2.0f, 0.0f));
            //		//	GRenderingComponents::decalManager().addComponent(decal_entity_4, "../resources/materials/decals/slatecliffrock_large.dmtl", 5.0f, 5.0f);
            //		//
            //		//});
            //
            //		Entity material_debug_entity = GEngineCore::entityManager().create();
            //		GRenderingComponents::materialManager().addComponent(material_debug_entity, "../resources/materials/freepbr/octostone.slmtl");
            //
            //		//Entity ocean_entity = GEngineCore::entityManager().create();
            //		//GCoreComponents::transformManager().addComponent(ocean_entity, Vec3(0.0, -18, 0.0));
            //		//GCoreComponents::oceanManager().addComponent(ocean_entity, 20.0, 512, 256);
            //
            //		//	Entity planet_sphere_entity = GEngineCore::entityManager().create();
            //		//	GCoreComponents::transformManager().addComponent(planet_sphere_entity,Vec3(0.0,-6360000.0,0.0),Quat(),Vec3(6360000.0));
            //		//	GRenderingComponents::staticMeshManager().addComponent(planet_sphere_entity,"../resources/materials/dfr_debug.slmtl","../resources/meshes/sphere.fbx",false);
            //
            //		/*
            //		static Entity monkey = GEngineCore::entityManager().create();
            //		GCoreComponents::transformManager().addComponent(monkey, Vec3(10.0f, 0.0f, 0.0f), Quat(), Vec3(1.0f));
            //		//GCoreComponents::airplanePhysicsManager().addComponent(monkey, Vec3(0.0f, 0.0f, 150.0f), Vec3(0.0f), 15000.0f, 3000.0f, 22.0f);
            //		GRenderingComponents::staticMeshManager().addComponent(monkey, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/monkey.fbx");
            //		GRenderingComponents::pickingManager().addComponent(monkey, "../resources/materials/editor/picking.slmtl", "../resources/meshes/monkey.fbx");
            //		GTools::selectManager().addComponent(monkey, []() { GTools::transformTool().activate(); }, []() { GTools::transformTool().deactivate(); });
            //		//Controls::setActiveAircraft(&monkey);
            //		//GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(camera_entity), monkey);
            //		*/
            //
            //
            //		//Entity box = GEngineCore::entityManager().create();
            //		//GCoreComponents::transformManager().addComponent(box, Vec3(0.0f, 0.0f, 0.0f), Quat(), Vec3(1.0f));
            //		//GRenderingComponents::staticMeshManager().addComponent(box, "../resources/materials/freepbr/octostone.slmtl", "../resources/meshes/box.fbx");
            //
            //
            //		/*
            //		Entity cube = GEngineCore::entityManager().create();
            //		GCoreComponents::transformManager().addComponent(cube, Vec3(5.0f, 0.0f, 0.0f), Quat(), Vec3(5.0f));
            //		GRenderingComponents::staticMeshManager().addComponent(cube, "../resources/materials/freepbr/mahogfloor.slmtl", "../resources/meshes/cube.fbx");
            //		GRenderingComponents::pickingManager().addComponent(cube, "../resources/materials/editor/picking.slmtl", "../resources/meshes/cube.fbx");
            //		GTools::selectManager().addComponent(cube, [cube]() { GTools::transformTool().activate(); }, [cube]() { GTools::transformTool().deactivate(); });
            //		*/
            //
        }

    }
}