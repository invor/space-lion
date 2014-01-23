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

	/*
	/	Support for adding cameras and lights via message system will follow later on
	*/

	/*	Terrain debuggin */
	std::shared_ptr<Material> terrain_mtl;
	std::shared_ptr<Texture> terrain_heightmap;
	if(!(resourceMngr.createTexture2D("../resources/textures/fapra/jacks_secret_stache_eroded_mountains_512.ppm",terrain_heightmap)))
		std::cout << "Failed to create heightmap texture." << std::endl;
	if (!(resourceMngr.createMaterial("../resources/materials/fapra/default_terrain.slmtl", terrain_mtl)))
		std::cout << "Failed to create material." << std::endl;
	if(!(demo_scene.loadTerrain(512, terrain_mtl, terrain_heightmap)))
		std::cout << "Failed to load terrain." << std::endl;

	terrain_mtl.reset();
	terrain_heightmap.reset();
	
	if(!(demo_scene.createSceneCamera(0,glm::vec3(0.0,0.0,5.0),glm::vec3(0.0,0.0,0.0),16.0f/9.0f,(9.0f/16.0f)*60.0f)))
		std::cout<<"Failed to create camera"<<"\n";

	if(!(demo_scene.createSceneLight(0,glm::vec3(256.0,256.0,256.0),glm::vec3(50.0))))
		std::cout<<"Failed to create light"<<"\n";
	
	demo_scene.setActiveCamera(0);


	//	/*	TEMPORARY SHADER TESTING */
	//	std::shared_ptr<Mesh> geomPtr;
	//	std::shared_ptr<Material> matPtr;
	//	resourceMngr.createBox(geomPtr);
	//	//resourceMngr.createMesh("../resources/meshes/maya_box.fbx", geomPtr);
	//	if (!(resourceMngr.createMaterial("../resources/materials/default.slmtl", matPtr)))
	//		std::cout << "Failed to create material." << std::endl;
	//	if (!(activeScene->createStaticSceneObject(1, glm::vec3(0.0,0.0,0.0), glm::quat(),glm::vec3(1.0), geomPtr, matPtr)))
	//		std::cout << "Failed to create scene object." << std::endl;
	//	
	//	geomPtr.reset();
	//	matPtr.reset();
	//	
	//	if(!(activeScene->createSceneCamera(0,glm::vec3(0.0,0.0,5.0),glm::vec3(0.0,0.0,0.0),16.0f/9.0f,(9.0f/16.0f)*60.0f)))
	//		std::cout<<"Failed to create camera"<<"\n";
	//	
	//	if(!(activeScene->createSceneLight(0,glm::vec3(5.0,2.5,1.5),glm::vec3(1.0))))
	//		std::cout<<"Failed to create light"<<"\n";
	//	
	//	activeScene->setActiveCamera(0);

	running = true;
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable( GL_MULTISAMPLE );

	while(running && !glfwWindowShouldClose(activeWindow))
	{
		/*	For now, I avoid using a glfw callback function for this */
		Controls::updateCamera(activeWindow, demo_scene.getActiveCamera());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		demo_scene.renderTerrain();

		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}

	/*	Make sure to delete all OpenGL resources while the context is still active */
	sceneList.clear();
	resourceMngr.clearLists();

	glfwDestroyWindow(activeWindow);
}