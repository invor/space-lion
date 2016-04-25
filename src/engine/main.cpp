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
#include "SunlightComponent.hpp"
#include "InterfaceMeshComponent.hpp"

//	#include "Terrain.hpp"

int main(){

	EntityManager						entity_mngr;
	ResourceManager						gfx_resource_mngr;
	TransformComponentManager			scene_transformations_mngr(1000000);
	CameraComponentManager				camera_mngr(10);
	PointlightComponentManager			light_mngr(10000);
	SunlightComponentManager			sunlight_mngr(10);
	AirplanePhysicsComponentManager		airplanePhysics_mngr(10000, &scene_transformations_mngr);
	AtmosphereComponentManager			atmosphere_mngr(100, &gfx_resource_mngr);
	StaticMeshComponentManager			staticMesh_mngr(&gfx_resource_mngr);
	VolumeComponentManager				volume_mngr;
	InterfaceMeshComponentManager		interfaceMesh_mngr(&gfx_resource_mngr);
	DeferredRenderingPipeline			rendering_pipeline(&entity_mngr,
															&gfx_resource_mngr,
															&scene_transformations_mngr,
															&camera_mngr,
															&light_mngr,
															&sunlight_mngr,
															&atmosphere_mngr,
															&staticMesh_mngr,
															&volume_mngr,
															&interfaceMesh_mngr);

	//	Landscape::FeatureCurveComponentManager landscapeFeatureCurve_mngr(&entity_mngr,&scene_transformations_mngr,&gfx_resource_mngr,&staticMesh_mngr,&interfaceMesh_mngr,&rendering_pipeline);
	//	Landscape::LandscapeBrickComponentManager landscaleBrick_mngr(&scene_transformations_mngr,&gfx_resource_mngr,&interfaceMesh_mngr,&staticMesh_mngr,&volume_mngr,&landscapeFeatureCurve_mngr,&rendering_pipeline);
	//	
	//	Landscape::StaticLandscapeComponentManager landscape_mngr(&entity_mngr,&scene_transformations_mngr,&landscaleBrick_mngr,&landscapeFeatureCurve_mngr);
	
	std::thread renderThread(&(DeferredRenderingPipeline::run),&rendering_pipeline);
	
	// Create test scene hard coded

	Entity camera_entity = entity_mngr.create();
	scene_transformations_mngr.addComponent(camera_entity,Vec3(0.0,0.0,20.0),Quat(),Vec3(1.0));
	camera_mngr.addComponent(camera_entity);// THIS IS NOT THE ACTIVE CAMERA ATM

	Entity sun_entity = entity_mngr.create();
	scene_transformations_mngr.addComponent(sun_entity,Vec3(0.0,149600000000.0,0.0),Quat(),Vec3(1.0));
	//scene_transformations_mngr.addComponent(sun_entity,Vec3(0.0,105783174466.0,105783174466.0),Quat(),Vec3(1.0));
	sunlight_mngr.addComponent(sun_entity,Vec3(1.0), 3.75f *std::pow(10.0,28.0), 150000000000.0 );

	//Entity sun_entity_2 = entity_mngr.create();
	//scene_transformations_mngr.addComponent(sun_entity_2,Vec3(149600000000.0,149600000.0,0.0),Quat(),Vec3(1.0));
	//sunlight_mngr.addComponent(sun_entity_2,Vec3(1.0), 3.75f *std::pow(10.0,28.0), 150000000000.0 );


	//	for(int j=-10; j < 10; j = j+1)
	//	{
	//		Entity light_entity = entity_mngr.create();
	//		scene_transformations_mngr.addComponent(light_entity,Vec3(0.0,(float)j, (float)j ),Quat(),Vec3(1.0));
	//		light_mngr.addComponent(light_entity,Vec3(1.0),1200.0,1000.0);
	//	}


	Entity atmoshphere_entity = entity_mngr.create();
	scene_transformations_mngr.addComponent(atmoshphere_entity,Vec3(0.0,-6361000.0,0.0),Quat(),Vec3(6420000.0)); // check the correct size for the earth's atmosphere
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

	//	Entity planet_sphere_entity = entity_mngr.create();
	//	scene_transformations_mngr.addComponent(planet_sphere_entity,Vec3(0.0,-6361000.0,0.0),Quat(),Vec3(6361000.0));
	//	staticMesh_mngr.addComponent(planet_sphere_entity,"../resources/materials/templates/gold.slmtl","../resources/meshes/sphere.fbx",false);
	//	
	//	Entity monkey = entity_mngr.create();
	//	scene_transformations_mngr.addComponent(monkey,Vec3(0.0,0.0,-20.0),Quat(),Vec3(2.0));
	//	airplanePhysics_mngr.addComponent(monkey,Vec3(0.0,0.0,80.0),Vec3(0.0),0.0,3000.0,22.0);
	//	staticMesh_mngr.addComponent(monkey,"../resources/materials/dfr_debug.slmtl","../resources/meshes/monkey.fbx",false);

	
	Entity debug_volume = entity_mngr.create();
	scene_transformations_mngr.addComponent(debug_volume,Vec3(0.0,0.0,-10.0),Quat(),Vec3(1.0));

	std::vector<float> debug_volume_data( {0.0,0.0,0.0,0.0, 1.0,1.0,1.0,1.0, 0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0,
											0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0, 1.0,1.0,1.0,1.0, 0.0,0.0,0.0,0.0,} );
	TextureDescriptor debug_volume_descriptor(GL_RGBA32F,2,2,2,GL_RGBA,GL_FLOAT);

	std::vector<float> debug_volume_bg_vertices( {-0.5,-0.5,-0.5,0.0,0.0,0.0,
													-0.5,0.5,-0.5,0.0,1.0,0.0,
													0.5,0.5,-0.5,1.0,1.0,0.0,
													0.5,-0.5,-0.5,1.0,0.0,0.0,
													-0.5,-0.5,0.5,0.0,0.0,1.0,
													-0.5,0.5,0.5,0.0,1.0,1.0,
													0.5,0.5,0.5,1.0,1.0,1.0,
													0.5,-0.5,0.5,1.0,0.0,1.0,} );
	std::vector<uint> debug_volume_bg_indices( {0,2,1,0,3,2, //back face
												0,1,4,1,5,4,
												4,5,7,7,5,6,
												7,6,3,3,2,6,
												5,1,6,6,1,2,
												4,7,0,0,7,3} );
	VertexDescriptor debug_volume_bg_vertexDesc(24,{VertexDescriptor::Attribute(GL_FLOAT,3,false,0),VertexDescriptor::Attribute(GL_FLOAT,3,false,3*sizeof(GLfloat))});

	//volume_mngr.addComponent(debug_volume,"debug_volume",debug_volume_data,debug_volume_descriptor,debug_volume_bg_vertices,debug_volume_bg_indices,debug_volume_bg_vertexDesc,GL_TRIANGLES);


	///////////////////////////////////////////////
	// LANDSCAPE DEBUGGING
	///////////////////////////////////////////////

	//	Entity debug_landscape = entity_mngr.create();
	//	landscape_mngr.addComponent(debug_landscape,Vec3(0.0f,0.0f,0.0f),Vec3(32.0f,32.0f,32.0f));
	
	//landscape_mngr.addFeatureCurve(0,Vec3(0.0,5.0,-10.0));
	//Entity cp = landscapeFeatureCurve_mngr.addConstraintPoint(0,0.4f);
	//landscapeFeatureCurve_mngr.setConstraintPointGradient(cp,Vec3(0.0,-1.0,0.0),1);
	//Entity cp1 = landscapeFeatureCurve_mngr.addConstraintPoint(0,0.6f);
	//landscapeFeatureCurve_mngr.setConstraintPointGradient(cp1,Vec3(0.0,-1.0,0.0),1);
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(16.0,0.0,0.0));

	//	landscape_mngr.addFeatureCurve(0,Vec3(0.3,0.3,-10.3));
	//	landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(10.0,0.0,2.0));
	//	landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(10.0,0.0,12.0));

	//landscape_mngr.addFeatureCurve(0,Vec3(0.3,0.3,10.3),glm::rotate(Quat(),3.14159265359f,Vec3(0.0,1.0,0.0)));
	//	landscape_mngr.addFeatureCurve(0,Vec3(0.3,0.3,10.3),glm::rotate(Quat(),3.14159f,Vec3(0.0,1.0,0.0)));
	//	landscapeFeatureCurve_mngr.addControlVertex(1,Vec3(10.0,0.0,2.0));
	//	landscapeFeatureCurve_mngr.addControlVertex(1,Vec3(10.0,0.0,12.0));

	//	landscaleBrick_mngr.updateBrick(0);

	//landscape_mngr.addFeatureCurve(0,Vec3(0.3,-10.0,-10.3));

	//landscape_mngr.addFeatureCurve(0,Vec3(5.0,0.3,-2.0),glm::rotate(Quat(),0.0f,Vec3(0.0,1.0,0.0)));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(2.0, 0.0, 1.0));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(2.0, 0.0, 3.0));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(2.0, 0.0, 5.0));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(1.0, 0.0, 6.0));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(-1.0, 0.0, 6.0));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(-10.0, 0.0, 10.0));

	//landscape_mngr.addFeatureCurve(0,Vec3(-6.3,0.3,4.0),glm::rotate(Quat(),3.14f,Vec3(0.0,1.0,0.0)));
	//landscape_mngr.addFeatureCurve(0,Vec3(-6.3,0.3,4.0),glm::rotate(Quat(),0.0f,Vec3(0.0,1.0,0.0)));
	//landscapeFeatureCurve_mngr.addControlVertex(0,Vec3(2.0, 0.0, 0.0));

	/*

	Vec3 curvePoint_position = landscapeFeatureCurve_mngr.calculateCurvePoint(0,1.0f);

	Entity debugCurvePoint = entity_mngr.create();
	scene_transformations_mngr.addComponent(debugCurvePoint,curvePoint_position,Quat(),Vec3(0.5));
	staticMesh_mngr.addComponent(debugCurvePoint,"../resources/materials/dfr_debug.slmtl","../resources/meshes/monkey.fbx",false);

	
	float curve_pos = 0.0f;
	bool increment = true;
	uint debugCurvePoint_transform_idx = scene_transformations_mngr.getIndex(debugCurvePoint);
	
	double dt = 0.0;

	while(true)
	{
		auto t_0 = std::chrono::high_resolution_clock::now();

		std::this_thread::sleep_until(t_0 + std::chrono::milliseconds(16));

		if(increment)
			curve_pos += 0.5 * dt;
		else
			curve_pos -= 0.5 * dt;

		//std::cout<<"Time: "<<dt<<std::endl;

		if(curve_pos > 1.0)
		{
			increment = false;
			curve_pos = 0.99999;
		}
		else if(curve_pos < 0.0)
		{
			increment = true;
			curve_pos = 0.000001;
		}

		Vec3 curvePoint_position = landscapeFeatureCurve_mngr.calculateCurvePoint(0,curve_pos);

		scene_transformations_mngr.setPosition(debugCurvePoint_transform_idx,curvePoint_position);

		auto t_1 = std::chrono::high_resolution_clock::now();

		dt = std::chrono::duration_cast<std::chrono::duration<double>>(t_1-t_0).count();
	}
	*/


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