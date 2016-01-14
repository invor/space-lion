#include <thread>
#include <vector>
#include <thread>
#include <future>
#include <chrono>

//#include "core/renderHub.h"

#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "AirplanePhysicsComponent.hpp"
#include "RenderingPipeline.hpp"
#include "DeferredRenderingPipeline.hpp"
#include "AdvancedDeferredRenderingPipeline.hpp"
#include "StaticMeshComponent.hpp"

int main(){

	EntityManager						entity_mngr;
	ResourceManager						gfx_resource_mngr;
	TransformComponentManager			scene_transformations_mngr(1000000);
	CameraComponentManager				camera_mngr(1000000);
	PointlightComponentManager			light_mngr(1000000);
	AirplanePhysicsComponentManager		airplanePhysics_mngr(10000);
	airplanePhysics_mngr.connectToTransformComponents(&scene_transformations_mngr);
	AtmosphereComponentManager			atmosphere_mngr(100,&gfx_resource_mngr);
	StaticMeshComponentManager			staticMesh_mngr;
	
	DeferredRenderingPipeline			rendering_pipeline(&entity_mngr,
															&gfx_resource_mngr,
															&scene_transformations_mngr,
															&camera_mngr,&light_mngr,
															&atmosphere_mngr,
															&staticMesh_mngr);
	
	std::thread renderThread(&(DeferredRenderingPipeline::run),&rendering_pipeline);
	
	// Create test scene hard coded

	Entity camera_entity = entity_mngr.create();
	scene_transformations_mngr.addComponent(camera_entity,Vec3(0.0,0.0,20.0),Quat(),Vec3(1.0));
	camera_mngr.addComponent(camera_entity);// THIS IS NOT THE ACTIVE CAMERA ATM

	Entity sun_entity = entity_mngr.create();
	scene_transformations_mngr.addComponent(sun_entity,Vec3(0.0,149600000000.0,0.0),Quat(),Vec3(1.0));
	light_mngr.addComponent(sun_entity,Vec3(1.0), 3.75f *std::pow(10.0,28.0), 150000000000.0 );
	rendering_pipeline.addLightsource(sun_entity);

	//	Entity light_entity = entity_mngr.create();
	//	scene_transformations_mngr.addComponent(light_entity,Vec3(10.0,10.0,10.0),Quat(),Vec3(1.0));
	//	light_mngr.addComponent(light_entity,Vec3(1.0),1200.0);
	//	rendering_pipeline.addLightsource(light_entity);


	Entity atmoshphere_entity = entity_mngr.create();
	scene_transformations_mngr.addComponent(atmoshphere_entity,Vec3(0.0,-6361000.0,0.0),Quat(),Vec3(6420000.0*2.0));
	atmosphere_mngr.addComponent(atmoshphere_entity,
									//Vec3(0.0058,0.0135,0.0331),
									//Vec3(0.00444,0.00444,0.00444),
									
									Vec3(0.0000058,0.0000135,0.0000331),
									Vec3(0.00000444,0.00000444,0.00000444),
									//Vec3(0.00002,0.00002,0.00002),
									8000.0,
									1200.0,
									6360000.0,
									6420000.0);
	
	// Stress-testing
	//	for(int j=-10; j < 10; j = j+4)
	//	{
	//		for(int i=-10; i < 10; i = i+10)
	//		{
	//			Entity aircraft_entity = entity_mngr.create();
	//			scene_transformations_mngr.addComponent(aircraft_entity,Vec3((float)i,(float)j,0.0),Quat(),Vec3(1.0));
	//			rendering_pipeline.requestRenderJob(aircraft_entity,"../resources/materials/debug.slmtl","../resources/meshes/outflyer.fbx");
	//		}
	//	}

	//Entity boat = entity_mngr.create();
	//scene_transformations_mngr.addComponent(boat,Vec3(0.0),Quat(),Vec3(1.0));
	//rendering_pipeline.requestRenderJob(boat,"../resources/materials/boat.slmtl","../resources/meshes/boat.fbx");
	
	//	Entity m4 = entity_mngr.create();
	//	scene_transformations_mngr.addComponent(m4,Vec3(0.0,0.0,-20.0),Quat(),Vec3(2.0));
	//	rendering_pipeline.requestRenderJob(RenderJobRequest(m4,"../resources/materials/m4.slmtl","../resources/meshes/m4.fbx"));
	//	
	//	Entity sponza = entity_mngr.create();
	//	scene_transformations_mngr.addComponent(sponza,Vec3(0.0),Quat(),Vec3(1.0));
	//	rendering_pipeline.requestRenderJob(RenderJobRequest(sponza,"../resources/materials/dfr_debug.slmtl","../resources/meshes/sponza_cust.fbx"));
	//	
	Entity airplane = entity_mngr.create();
	scene_transformations_mngr.addComponent(airplane,Vec3(0.0,0.0,0.0),Quat(),Vec3(1.0));
	airplanePhysics_mngr.addComponent(airplane,Vec3(0.0,0.0,80.0),Vec3(0.0),0.0,3000.0,22.0);
	staticMesh_mngr.addComponent(airplane,"../resources/materials/dfr_debug.slmtl","../resources/meshes/outflyer.fbx",false);

	//rendering_pipeline.requestRenderJob(RenderJobRequest(airplane,"../resources/materials/dfr_debug.slmtl","../resources/meshes/outflyer.fbx"));
	
	//	Entity toad = entity_mngr.create();
	//	scene_transformations_mngr.addComponent(toad,Vec3(0.0,0.0,0.0),Quat(),Vec3(1.1));
	//	rendering_pipeline.requestRenderJob(RenderJobRequest(toad,"../resources/materials/toad.slmtl","../resources/meshes/toad.fbx"));
	

	//	while(true)
	//	{
	//		auto t_0 = std::chrono::high_resolution_clock::now();
	//	
	//		std::this_thread::sleep_until(t_0 + std::chrono::milliseconds(5));
	//	
	//		auto t_1 = std::chrono::high_resolution_clock::now();
	//	
	//		airplanePhysics_mngr.update( std::chrono::duration_cast<std::chrono::duration<double>>(t_1-t_0).count() );
	//	
	//		std::cout<<scene_transformations_mngr.getPosition(scene_transformations_mngr.getIndex(airplane)).x<<" "
	//			     <<scene_transformations_mngr.getPosition(scene_transformations_mngr.getIndex(airplane)).y<<" "
	//				 <<scene_transformations_mngr.getPosition(scene_transformations_mngr.getIndex(airplane)).z<<std::endl;
	//	}
	
	//	while(true)
	//	{
	//		auto t_0 = std::chrono::high_resolution_clock::now();
	//	
	//		std::this_thread::sleep_until(t_0 + std::chrono::milliseconds(5));
	//	
	//		scene_transformations_mngr.translate(scene_transformations_mngr.getIndex(light_entity), Vec3(0.0,0.0,-0.1));
	//	
	//		auto t_1 = std::chrono::high_resolution_clock::now();
	//	}


	renderThread.join();
	

	return 0;
}
