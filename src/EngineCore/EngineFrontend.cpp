#include "EngineFrontend.hpp"

#include <thread>
#include <vector>
#include <future>
#include <chrono>

//#include "OpenGL/BasicRenderingPipeline.hpp"
#include "OpenGL/GraphicsBackend.hpp"
#include "OpenGL/ResourceManager.hpp"
#include "TaskSchedueler.hpp"
#include "WorldState.hpp"

namespace EngineCore
{
	namespace Common
	{
		void EngineFrontend::startEngine()
		{
			// create task manager
			Utility::TaskSchedueler task_schedueler;

			// create frame manager
			FrameManager frame_manager;

			// create graphics backend and graphics resource management
			Graphics::OpenGL::GraphicsBackend graphics_backend;
			Graphics::OpenGL::ResourceManager resource_manager;

			// create world
			WorldState world_state(&resource_manager);


			// create initial frame
			size_t frameID = 0;

			// start rendering pipeline
			//std::thread render_thread(&(DeferredRenderingPipeline::run), &GEngineCore::renderingPipeline()));
			auto render_exec = std::async(std::launch::async, &(Graphics::OpenGL::GraphicsBackend::run), &graphics_backend, &resource_manager, &frame_manager);
			auto render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

			//auto render_exec = std::async(std::launch::async, &(GraphicsBackend::run), &GEngineCore::graphicsBackend());
			//auto render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

			// start task schedueler with 1 thread
			task_schedueler.run(1);

			auto t_0 = std::chrono::high_resolution_clock::now();
			auto t_1 = std::chrono::high_resolution_clock::now();

			// engine update loop
			while (render_exec_status != std::future_status::ready)
			{
				double dt = std::chrono::duration_cast<std::chrono::duration<double>>(t_1 - t_0).count();
				//std::cout << "dt: " << dt << std::endl;
				t_0 = std::chrono::high_resolution_clock::now();

				//TODO update world

				// finalize engine update by creating a new frame
				Frame new_frame;

				new_frame.m_frameID = frameID;
				new_frame.m_dt = dt;

				//uint camera_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
				//Entity camera_entity = GCoreComponents::cameraManager().getEntity(camera_idx);
				//uint camera_transform_idx = GCoreComponents::transformManager().getIndex(camera_entity);
				//new_frame.m_view_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(camera_transform_idx));
				//new_frame.m_projection_matrix = GCoreComponents::cameraManager().getProjectionMatrix(camera_idx);
				//new_frame.m_fovy = GCoreComponents::cameraManager().getFovy(camera_idx);
				//new_frame.m_aspect_ratio = GCoreComponents::cameraManager().getAspectRatio(camera_idx);
				//new_frame.m_exposure = GCoreComponents::cameraManager().getExposure(camera_idx);

				//Graphics::OpenGL::setupBasicRenderingPipeline(new_frame);

				frame_manager.swapUpdateFrame(new_frame);

				// check if rendering pipeline is still running
				render_exec_status = render_exec.wait_for(std::chrono::microseconds(0));

				t_1 = std::chrono::high_resolution_clock::now();
			}

			task_schedueler.stop();
		}

		void EngineFrontend::createDemoScene()
		{
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
	//		/*
	//		// Create monkey grid
	//		for (int z = -200; z <= 200; z += 10)
	//		{
	//			for (int y = -200; y <= 200; y += 10)
	//			{
	//				Entity grid_monkey = GEngineCore::entityManager().create();
	//				GCoreComponents::transformManager().addComponent(grid_monkey, Vec3(15.0f, (float)y, (float)z), Quat(), Vec3(1.0f));
	//				GRenderingComponents::staticMeshManager().addComponent(grid_monkey, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/monkey.fbx");
	//				//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([grid_monkey]() {
	//				//	std::vector<uint8_t> vertex_data;
	//				//	std::vector<uint32_t> index_data;
	//				//	VertexLayout vertex_layout;
	//				//	ResourceLoading::loadFbxGeometrySlim("../resources/meshes/monkey.fbx", vertex_data, index_data, vertex_layout);
	//				//	Mesh* mesh = GEngineCore::resourceManager().createMesh("monkey_slim",
	//				//		vertex_data,
	//				//		index_data,
	//				//		vertex_layout,
	//				//		GL_TRIANGLES).get();
	//				//	GRenderingComponents::shadowCastMeshManager().addComponent(grid_monkey, "../resources/materials/shadowCaster.slmtl", mesh);
	//				//}
	//				//);
	//			}
	//		}
	//		*/
	//
	//		/*
	//		Entity cube = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(cube, Vec3(5.0f, 0.0f, 0.0f), Quat(), Vec3(5.0f));
	//		GRenderingComponents::staticMeshManager().addComponent(cube, "../resources/materials/freepbr/mahogfloor.slmtl", "../resources/meshes/cube.fbx");
	//		GRenderingComponents::pickingManager().addComponent(cube, "../resources/materials/editor/picking.slmtl", "../resources/meshes/cube.fbx");
	//		GTools::selectManager().addComponent(cube, [cube]() { GTools::transformTool().activate(); }, [cube]() { GTools::transformTool().deactivate(); });
	//		*/
	//
	//		/*
	//		// Create monkey grid
	//		for (int z = -20; z <= 20; z += 10)
	//		{
	//			for (int y = -20; y <= 20; y += 10)
	//			{
	//				Entity grid_monkey = GEngineCore::entityManager().create();
	//				GCoreComponents::transformManager().addComponent(grid_monkey, Vec3(15.0f, (float)y, (float)z), Quat(), Vec3(1.0f));
	//				GRenderingComponents::staticMeshManager().addComponent(grid_monkey, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/monkey.fbx");
	//				GEngineCore::renderingPipeline().addSingleExecutionGpuTask([grid_monkey]() {
	//						std::vector<uint8_t> vertex_data;
	//						std::vector<uint32_t> index_data;
	//						VertexLayout vertex_layout;
	//						ResourceLoading::loadFbxGeometrySlim("../resources/meshes/monkey.fbx", vertex_data, index_data, vertex_layout);
	//						Mesh* mesh = GEngineCore::resourceManager().createMesh("monkey_slim",
	//							vertex_data,
	//							index_data,
	//							vertex_layout,
	//							GL_TRIANGLES).get();
	//						GRenderingComponents::shadowCastMeshManager().addComponent(grid_monkey, "../resources/materials/shadowCaster.slmtl", mesh);
	//					}
	//				);
	//			}
	//		}
	//
	//		// Create pointlight grid
	//		for (int z = -5; z <= 5; z += 5)
	//		{
	//			for (int y = -5; y <= 5; y += 5)
	//			{
	//				Entity light = GEngineCore::entityManager().create();
	//				GCoreComponents::transformManager().addComponent(light, Vec3(-10.0f, (float)y, (float)z));
	//				GCoreComponents::pointlightManager().addComponent(light, Vec3(1.0), 10000000, 1000);
	//				GRenderingComponents::interfaceMeshManager().addComponent(light, "../resources/materials/editor/interface_pointlight.slmtl", "../resources/meshes/editor/pointlight.fbx");
	//				GRenderingComponents::pickingManager().addComponent(light, "../resources/materials/editor/picking.slmtl", "../resources/meshes/editor/pointlight.fbx");
	//				GTools::selectManager().addComponent(light, [light]() { GTools::transformTool().activate(); }, [light]() { GTools::transformTool().deactivate(); });
	//				GCoreComponents::debugNameManager().addComponent(light, "light " + std::to_string(z) + std::to_string(y));
	//			}
	//		}
	//
	//		Entity cube = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(cube, Vec3(5.0f, 0.0f, 0.0f), Quat(), Vec3(5.0f));
	//		GRenderingComponents::staticMeshManager().addComponent(cube, "../resources/materials/freepbr/mahogfloor.slmtl", "../resources/meshes/cube.fbx");
	//		GRenderingComponents::pickingManager().addComponent(cube, "../resources/materials/editor/picking.slmtl", "../resources/meshes/cube.fbx");
	//		GTools::selectManager().addComponent(cube, [cube]() { GTools::transformTool().activate(); }, [cube]() { GTools::transformTool().deactivate(); });
	//
	//		Entity cube2 = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(cube2, Vec3(-10.0f, 0.0f, 0.0f), Quat(), Vec3(5.0f));
	//		GRenderingComponents::staticMeshManager().addComponent(cube2, "../resources/materials/freepbr/mahogfloor.slmtl", "../resources/meshes/cube.fbx");
	//		GRenderingComponents::pickingManager().addComponent(cube2, "../resources/materials/editor/picking.slmtl", "../resources/meshes/cube.fbx");
	//		GTools::selectManager().addComponent(cube2, [cube2]() { GTools::transformTool().activate(); }, [cube2]() { GTools::transformTool().deactivate(); });
	//
	//		Entity sphere = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(sphere, Vec3(0.0f, 0.0f, -10.0f), Quat(), Vec3(1.0f));
	//		GRenderingComponents::staticMeshManager().addComponent(sphere, "../resources/materials/freepbr/mahogfloor.slmtl", "../resources/meshes/sphere.fbx");
	//		GRenderingComponents::pickingManager().addComponent(sphere, "../resources/materials/editor/picking.slmtl", "../resources/meshes/cube.fbx");
	//		GTools::selectManager().addComponent(sphere, [sphere]() { GTools::transformTool().activate(); }, [sphere]() { GTools::transformTool().deactivate(); });
	//
	//		Entity texture_monkey = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(texture_monkey, Vec3(0.0f, 0.0f, 0.0f), Quat(), Vec3(1.0f));
	//		GRenderingComponents::staticMeshManager().addComponent(texture_monkey, "../resources/materials/freepbr/graniterockface1.slmtl", "../resources/meshes/monkey_sphereUV.fbx");
	//		GRenderingComponents::pickingManager().addComponent(texture_monkey, "../resources/materials/editor/picking.slmtl", "../resources/meshes/monkey_sphereUV.fbx");
	//		GTools::selectManager().addComponent(texture_monkey, [texture_monkey]() { GTools::transformTool().activate(); }, [texture_monkey]() { GTools::transformTool().deactivate(); });
	//
	//		Entity cerberus = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(cerberus, Vec3(0.0f, 0.0f, 0.0f), Quat(), Vec3(1.0f));
	//		GRenderingComponents::staticMeshManager().addComponent(cerberus, "../resources/materials/cerberus.slmtl", "../resources/meshes/Cerberus.fbx");
	//		GRenderingComponents::pickingManager().addComponent(cerberus, "../resources/materials/editor/picking.slmtl", "../resources/meshes/Cerberus.fbx");
	//		GTools::selectManager().addComponent(cerberus, [cerberus]() { GTools::transformTool().activate(); }, [cerberus]() { GTools::transformTool().deactivate(); });
	//		GCoreComponents::debugNameManager().addComponent(cerberus, "cerberus");
	//		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([cerberus]() {
	//			std::vector<uint8_t> vertex_data;
	//			std::vector<uint32_t> index_data;
	//			VertexLayout vertex_layout;
	//			ResourceLoading::loadFbxGeometrySlim("../resources/meshes/Cerberus.fbx", vertex_data, index_data, vertex_layout);
	//			Mesh* mesh = GEngineCore::resourceManager().createMesh("cerberus_slim",
	//				vertex_data,
	//				index_data,
	//				vertex_layout,
	//				GL_TRIANGLES).get();
	//			GRenderingComponents::shadowCastMeshManager().addComponent(cerberus, "../resources/materials/shadowCaster.slmtl", mesh);
	//		}
	//		);
	//		*/
	//
	//		Entity ground_plane = GEngineCore::entityManager().create();
	//		GCoreComponents::transformManager().addComponent(ground_plane, Vec3(0.0, -0.5, 0.0), Quat(), Vec3(1.0));
	//		auto plane_geometry = GeometryBakery::createPlane(2.0f, 2.0f);
	//		GRenderingComponents::staticMeshManager().addComponent(ground_plane, "ground_plane", "../resources/materials/dfr_checkerboard.slmtl", std::get<0>(plane_geometry), std::get<1>(plane_geometry), std::get<2>(plane_geometry), GL_TRIANGLES, true);
	//		//GRenderingComponents::staticMeshManager().addComponent(ground_plane, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/ground_plane.fbx", false);
	//
	//		auto ground_plane_mtl = GRenderingComponents::materialManager().addComponent(ground_plane, "../resources/materials/brp_simple.slmtl");
	//		auto ground_plane_mesh = GRenderingComponents::meshManager().addComponent(ground_plane, "ground_plane_200_200", std::get<0>(plane_geometry), std::get<1>(plane_geometry), std::get<2>(plane_geometry), GL_TRIANGLES, true);
	//		GRenderingComponents::simpleObjectDrawManager().addComponent(ground_plane, ground_plane_mesh, ground_plane_mtl);
	//
	//		//Entity debug_box = GEngineCore::entityManager().create();
	//		//GCoreComponents::transformManager().addComponent(debug_box, Vec3(-1.75, 0.0, -37.5), Quat(), Vec3(5.0));
	//		//auto obstacle = GeometryBakery::createBox();
	//		//GRenderingComponents::staticMeshManager().addComponent(debug_box, "box", "../resources/materials/dfr_checkerboard.slmtl", std::get<0>(obstacle), std::get<1>(obstacle), std::get<2>(obstacle), GL_TRIANGLES, true);
	//		//GTools::selectManager().addComponent(debug_box, []() { GTools::transformTool().activate(); }, []() { GTools::transformTool().deactivate(); });
	//		//GRenderingComponents::pickingManager().addComponent(debug_box, "box_picking", "../resources/materials/editor/picking.slmtl", std::get<0>(obstacle), std::get<1>(obstacle), std::get<2>(obstacle), GL_TRIANGLES);
	//
	//
	//		//Entity tragwerk = GEngineCore::entityManager().create();
	//		//GCoreComponents::transformManager().addComponent(tragwerk, Vec3(), Quat(), Vec3(1.0f));
	//		//GRenderingComponents::staticMeshManager().addComponent(tragwerk, "../resources/materials/templates/gold.slmtl", "../resources/meshes/tragwerk_gesamt.fbx");
	//		//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([tragwerk]() {
	//		//	std::vector<uint8_t> vertex_data;
	//		//	std::vector<uint32_t> index_data;
	//		//	VertexLayout vertex_layout;
	//		//	ResourceLoading::loadFbxGeometrySlim("../resources/meshes/tragwerk_gesamt.fbx", vertex_data, index_data, vertex_layout);
	//		//	Mesh* mesh = GEngineCore::resourceManager().createMesh("tragwerk_slim",
	//		//		vertex_data,
	//		//		index_data,
	//		//		vertex_layout,
	//		//		GL_TRIANGLES).resource;
	//		//	GRenderingComponents::shadowCastMeshManager().addComponent(tragwerk, "../resources/materials/shadowCaster.slmtl", mesh);
	//		//}
	//		//);
		}

	}
}