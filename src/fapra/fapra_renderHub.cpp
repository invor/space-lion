#include "fapra_renderHub.h"


FapraRenderHub::FapraRenderHub(void)
{
}

FapraRenderHub::~FapraRenderHub(void)
{
}

void FapraRenderHub::renderActiveScene()
{
	/*	
	/	Create resource needed for additional render passes, e.g. shader programs not related to object materials
	/	and framebuffer objects.
	/	Framebuffers are created and stored locally in the context of this method.
	*/
	//std::shared_ptr<GLSLProgram> picking_prgm;
	//resourceMngr.createShaderProgram(PICKING, picking_prgm);
	//
	//FramebufferObject picking_fbo(800,450,true,false);
	//picking_fbo.createColorAttachment(GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
	//

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

	/*	TEMPORARY SHADER TESTING */
	std::shared_ptr<Mesh> geomPtr;
	std::shared_ptr<Material> matPtr;
	//resourceMngr.createBox(geomPtr);
	//resourceMngr.createMesh("../resources/meshes/demo_hangar.fbx",geomPtr);
	//if(!(resourceMngr.createMaterial("../resources/materials/demo_hangar.slmtl",matPtr)))
	//	std::cout<<"Failed to create material."<<std::endl;
	//if(!(activeScene->createStaticSceneObject(0,glm::vec3(0.0,0.0,0.0),glm::quat(),glm::vec3(1.0),geomPtr,matPtr)))
	//	std::cout<<"Failed to create scene object."<<std::endl;
	//
	//geomPtr.reset();
	//matPtr.reset();
	
	resourceMngr.createMesh("../resources/meshes/terrain.fbx", geomPtr);
	if (!(resourceMngr.createMaterial("../resources/materials/default.slmtl", matPtr)))
		std::cout << "Failed to create material." << std::endl;
	if (!(activeScene->createStaticSceneObject(1, glm::vec3(0.0,0.0, 0.0), glm::quat(),glm::vec3(5000.0), geomPtr, matPtr)))
		std::cout << "Failed to create scene object." << std::endl;
	
	geomPtr.reset();
	matPtr.reset();

	if(!(activeScene->createSceneCamera(0,glm::vec3(0.0,2000.0,5.0),glm::vec3(0.0,2000.0,0.0),16.0f/9.0f,(9.0f/16.0f)*60.0f)))
		std::cout<<"Failed to create camera"<<"\n";

	if(!(activeScene->createSceneLight(0,glm::vec3(2500.0,2500.0,1500.0),glm::vec3(150000.0))))
		std::cout<<"Failed to create light"<<"\n";

	activeScene->setActiveCamera(0);

	running = true;
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable( GL_MULTISAMPLE );

	/*  Test picking pass */
	GLuint *data = new GLuint[1];

	while(running && !glfwWindowShouldClose(activeWindow))
	//while(running)
	{
		/*	For now, I avoid using a glfw callback function for this */
		Controls::updateCamera(activeWindow, activeScene->getActiveCamera());

		/*  Test picking pass */
		//picking_fbo.bind();
		//glClearColor(0, 0, 0, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport(0, 0, picking_fbo.getWidth(), picking_fbo.getHeight());
		//activeScene->drawPicking(picking_prgm);
		//glReadBuffer(GL_COLOR_ATTACHMENT0);
		//glReadPixels(400, 225, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
		//std::cout << data[0] << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		activeScene->drawFroward();

		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}

	/*	Make sure to delete all OpenGL resources while the context is still active */
	sceneList.clear();
	resourceMngr.clearLists();

	glfwDestroyWindow(activeWindow);
}