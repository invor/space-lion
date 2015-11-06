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

using namespace std;

int main(){

	EntityManager entity_mngr;
	TransformComponentManager scene_transformations_mngr(1000000);
	CameraComponentManager camera_mngr(1000000);
	LightComponentManager light_mngr(1000000);
	AirplanePhysicsComponentManager airplanePhysics_mngr(10000);
	airplanePhysics_mngr.connectToTransformComponents(&scene_transformations_mngr);
	
	DeferredRenderingPipeline rendering_pipeline(&entity_mngr, &scene_transformations_mngr, &camera_mngr,&light_mngr);
	
	std::thread renderThread(&(DeferredRenderingPipeline::run),&rendering_pipeline);
	
	// Create test scene hard coded
	Entity camera_entity = entity_mngr.create();
	Entity light_entity = entity_mngr.create();
	
	scene_transformations_mngr.addComponent(camera_entity,Vec3(0.0,0.0,20.0),Quat(),Vec3(1.0));
	scene_transformations_mngr.addComponent(light_entity,Vec3(250.0,250.0,250.0),Quat(),Vec3(1.0));
	
	camera_mngr.addComponent(camera_entity);
	
	light_mngr.addComponent(light_entity);
	
	rendering_pipeline.addLightsource(light_entity);
	
	
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
	
	Entity m4 = entity_mngr.create();
	scene_transformations_mngr.addComponent(m4,Vec3(0.0),Quat(),Vec3(2.0));
	rendering_pipeline.requestRenderJob(m4,"../resources/materials/m4.slmtl","../resources/meshes/m4.fbx");

	//Entity sponza = entity_mngr.create();
	//scene_transformations_mngr.addComponent(sponza,Vec3(0.0),Quat(),Vec3(1.0));
	//rendering_pipeline.requestRenderJob(sponza,"../resources/materials/dfr_debug.slmtl","../resources/meshes/sponza_cust.fbx");

	Entity airplane = entity_mngr.create();
	scene_transformations_mngr.addComponent(airplane,Vec3(0.0,0.0,40.0),Quat(),Vec3(1.0));
	airplanePhysics_mngr.addComponent(airplane,Vec3(0.0,0.0,80.0),Vec3(0.0),0.0,3000.0,22.0);
	rendering_pipeline.requestRenderJob(airplane,"../resources/materials/dfr_debug.slmtl","../resources/meshes/outflyer.fbx");
	
	/*
	while(true)
	{
		auto t_0 = std::chrono::high_resolution_clock::now();

		std::this_thread::sleep_until(t_0 + std::chrono::milliseconds(5));

		auto t_1 = std::chrono::high_resolution_clock::now();

		airplanePhysics_mngr.update( std::chrono::duration_cast<std::chrono::duration<double>>(t_1-t_0).count() );

		std::cout<<scene_transformations_mngr.getPosition(scene_transformations_mngr.getIndex(airplane)).x<<" "
			     <<scene_transformations_mngr.getPosition(scene_transformations_mngr.getIndex(airplane)).y<<" "
				 <<scene_transformations_mngr.getPosition(scene_transformations_mngr.getIndex(airplane)).z<<std::endl;
	}
	*/


	renderThread.join();
	

	return 0;
}
