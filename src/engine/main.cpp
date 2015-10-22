#include <thread>
#include <vector>
#include <thread>
#include <future>

//#include "core/renderHub.h"

#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "RenderingPipeline.hpp"
#include "DeferredRenderingPipeline.hpp"

using namespace std;

int main(){

	EntityManager entity_mngr;
	TransformComponentManager scene_transformations_mngr(1000000);
	CameraComponentManager camera_mngr(1000000);
	LightComponentManager light_mngr(1000000);
	
	DeferredRenderingPipeline rendering_pipeline(&entity_mngr, &scene_transformations_mngr, &camera_mngr,&light_mngr);
	
	std::thread renderThread(&(DeferredRenderingPipeline::run),&rendering_pipeline);
	
	// Create test scene hard coded
	Entity camera_entity = entity_mngr.create();
	Entity light_entity = entity_mngr.create();
	
	scene_transformations_mngr.addComponent(camera_entity);
	scene_transformations_mngr.addComponent(light_entity,Vec3(250.0,250.0,150.0),Quat(),Vec3(1.0));
	
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

	Entity sponza = entity_mngr.create();
	scene_transformations_mngr.addComponent(sponza,Vec3(0.0),Quat(),Vec3(1.0));
	rendering_pipeline.requestRenderJob(sponza,"../resources/materials/dfr_debug.slmtl","../resources/meshes/sponza_cust.fbx");
	
	renderThread.join();
	

	return 0;
}
