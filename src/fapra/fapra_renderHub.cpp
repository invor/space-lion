#include "fapra_renderHub.h"


FapraRenderHub::FapraRenderHub(void)
{
}

FapraRenderHub::~FapraRenderHub(void)
{
}

void FapraRenderHub::renderActiveScene()
{
	glfwMakeContextCurrent(activeWindow);

	PostProcessor post_proc(800, 450);
	if (!post_proc.init(&resourceMngr))
	{
		std::cout << "Failed to create post processor"
			<< "\n";
	}

	/*	Create framebuffers for terrain, sky and compositing */

	terrain_fbo = std::shared_ptr<FramebufferObject>(new FramebufferObject(1366,768,true,false));
	/*	diffuse albedo */
	if(!terrain_fbo->createColorAttachment(GL_RGB8,GL_RGB,GL_BYTE))
	{
		std::cout<<"Failed to create terrain diffuse albedo colour attachment"<<std::endl;
		return;
	}
	/*	Normal.x,Normal.y,Tanget.x,Tanget.y */
	if(!terrain_fbo->createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT))
	{
		std::cout<<"Failed to create terrain normal and tangent attachment"<<std::endl;
		return;
	}
	/*	specular color, roughness */
	if(!terrain_fbo->createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT))
	{
		std::cout<<"Failed to create terrain specular colour and roughness attachment"<<std::endl;
		return;
	}
	/*	(linear) depth */
	if(!terrain_fbo->createColorAttachment(GL_R32F,GL_RED,GL_FLOAT))
	{
		std::cout<<"Failed to create terrain depth attachment"<<std::endl;
		return;
	}

	sky_fbo = std::shared_ptr<FramebufferObject>(new FramebufferObject(1366,768,true,false));
	if(!sky_fbo->createColorAttachment(GL_RGB8,GL_RGB,GL_BYTE))
	{
		std::cout<<"Failed to create sky colour attachment"<<std::endl;
		return;
	}

	/*
	/	Support for adding cameras and lights via message system will follow later on
	*/

	/*	Terrain & Sky debuggin */
	float time_of_day = 0.0;
	std::shared_ptr<Material> terrain_mtl;
	std::shared_ptr<Texture> terrain_heightmap;
	if(!(resourceMngr.createTexture2D("../resources/textures/fapra/terrain_heightmap.ppm",terrain_heightmap))){
		std::cout << "Failed to create heightmap texture." << std::endl;
		return;
	}
	if (!(resourceMngr.createMaterial("../resources/materials/fapra/default_terrain.slmtl", terrain_mtl))){
		std::cout << "Failed to create material." << std::endl;
		return;
	}
	if(!(demo_scene.loadTerrain(32, 12, terrain_mtl, terrain_heightmap))){
		std::cout << "Failed to load terrain." << std::endl;
		return;
	}
	if(!(demo_scene.initAtmosphere(&resourceMngr))){
		std::cout << "Failed to create atmosphere." << std::endl;
		return;
	}

	terrain_mtl.reset();
	terrain_heightmap.reset();
	
	if(!(demo_scene.createSceneCamera(0,glm::vec3(0.0,0.0,1.0),glm::vec3(0.0,0.0,0.0),16.0f/9.0f,(9.0f/16.0f)*60.0f)))
		std::cout<<"Failed to create camera"<<"\n";

	if(!(demo_scene.createSceneLight(0,glm::vec3(512.0,512.0,512.0),glm::vec3(500.0))))
		std::cout<<"Failed to create light"<<"\n";
	
	demo_scene.setActiveCamera(0);


	/*	TEMPORARY SHADER TESTING */
	//std::shared_ptr<Mesh> geomPtr;
	//std::shared_ptr<Material> matPtr;
	//resourceMngr.createBox(geomPtr);
	////resourceMngr.createMesh("../resources/meshes/maya_box.fbx", geomPtr);
	//if (!(resourceMngr.createMaterial("../resources/materials/default.slmtl", matPtr)))
	//	std::cout << "Failed to create material." << std::endl;
	//if (!(activeScene->createStaticSceneObject(1, glm::vec3(0.0,0.0,0.0), glm::quat(),glm::vec3(1.0), geomPtr, matPtr)))
	//	std::cout << "Failed to create scene object." << std::endl;
	//
	//geomPtr.reset();
	//matPtr.reset();
	//
	//if(!(activeScene->createSceneCamera(0,glm::vec3(0.0,0.0,5.0),glm::vec3(0.0,0.0,0.0),16.0f/9.0f,(9.0f/16.0f)*60.0f)))
	//	std::cout<<"Failed to create camera"<<"\n";
	//
	//if(!(activeScene->createSceneLight(0,glm::vec3(5.0,2.5,1.5),glm::vec3(1.0))))
	//	std::cout<<"Failed to create light"<<"\n";
	//
	//activeScene->setActiveCamera(0);

	running = true;
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable( GL_MULTISAMPLE );

	while(running && !glfwWindowShouldClose(activeWindow))
	{
		/*	For now, I avoid using a glfw callback function for this */
		Controls::updateCamera(activeWindow, demo_scene.getActiveCamera());
		//Controls::updateCamera(activeWindow, activeScene->getActiveCamera());

		//activeScene->drawFroward();

		terrain_fbo->bind();
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0,0,terrain_fbo->getWidth(),terrain_fbo->getHeight());
		demo_scene.renderTerrain();

		sky_fbo->bind();
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0,0,sky_fbo->getWidth(),sky_fbo->getHeight());
		time_of_day = glfwGetTime()*24.0 - 1440.0f * std::floor(time_of_day/1440.0f);
		demo_scene.renderSky(&post_proc,time_of_day,&(*terrain_fbo));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);
		post_proc.FBOToFBO(&(*sky_fbo));

		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}

	/*	Make sure to delete all OpenGL resources while the context is still active */
	sceneList.clear();
	resourceMngr.clearLists();

	glfwDestroyWindow(activeWindow);
}