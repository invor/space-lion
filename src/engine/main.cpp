#include <thread>
#include <vector>
#include <thread>
#include <future>

#include "core/renderHub.h"

#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "RenderingPipeline.hpp"

using namespace std;

int main(){

	
	//RenderHub testRenderer;
	//if(testRenderer.init())
	//{
	//	testRenderer.addScene();
	//	testRenderer.setActiveScene(0);
	//	testReceiver->pushLoadSceneMessages();
	//	std::thread renderThread(&RenderHub::run,&testRenderer);
	//}
	//testRenderer.init();
	//testRenderer.run();
	//std::thread renderThread(&RenderHub::run,&testRenderer);

	//renderThread.join();


	EntityManager entity_mngr;
	TransformComponentManager scene_transformations_mngr(1000000);
	CameraComponentManager camera_mngr(1000000);
	RenderingPipeline rendering_pipeline(&entity_mngr, &scene_transformations_mngr, &camera_mngr);
	
	std::thread renderThread(&(RenderingPipeline::run),&rendering_pipeline);

	/* Create test scene hard coded */
	Entity camera_entity = entity_mngr.create();
	Entity aircraft_entity = entity_mngr.create();
	Entity light_entity = entity_mngr.create();
	
	scene_transformations_mngr.addComponent(camera_entity);
	scene_transformations_mngr.addComponent(aircraft_entity);
	scene_transformations_mngr.addComponent(light_entity);
	
	camera_mngr.addComponent(camera_entity);

	//	rendering_pipeline.requestRenderJob(aircraft_entity,"../resources/materials/debug.slmtl","../resources/meshes/outflyer.fbx");

	renderThread.join();

	return 0;
}
