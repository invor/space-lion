#include "renderHub.h"


renderHub::renderHub(void)
{
}

renderHub::~renderHub(void)
{
}


bool renderHub::init()
{
	//	Initialize GLFW
	if(!glfwInit())
	{
		return false;
	}
	std::cout<<"Initializing GLFW\n";

	//#ifdef _WIN32
	//	//	Get highest openGL Version (doesn't work on linux right now)
	//	//	The glfwGetGLVersion function seems somewhat broken on windows too. When certain, seemingly random
	//	//	conditions are met.
	//
	//	int maj, min, rev;
	//    glfwGetGLVersion(&maj, &min, &rev);
	//
	//	//	Open a glfw window
	//	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, maj);
	//	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, min);
	//	//glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//	//glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//#else
	//	//	Better be save than sorry and choose a low GL version on linux
	//	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	//	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
	//#endif

	int maj, min;

	#ifdef _WIN32
		maj = 3;
		min = 3;
	#else
		maj = 2;
		min = 0;
	#endif

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, maj);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, min);

	if(!glfwOpenWindow(700,700,8,8,8,8,32,0,GLFW_WINDOW))
	{
		std::cout<<"-----\n"
				<<"The time is out of joint - O cursed spite,\n"
				<<"That ever I was born to set it right!\n"
				<<"-----\n"
				<<"Error: Couldn't open glfw window";
		return false;
	}

	const GLubyte *version = glGetString(GL_VERSION);
	std::cout<<"Supporting OpenGL Version: "<<version<<"\n";
	std::cout<<"Using OpenGL Version: "<<maj<<"."<<min<<"\n\n";

	//	Initialize glew
	GLenum error = glewInit();
	if( GLEW_OK != error)
	{
		std::cout<<"-----\n"
				<<"The time is out of joint - O cursed spite,\n"
				<<"That ever I was born to set it right!\n"
				<<"-----\n"
				<<"Error: "<<glewGetErrorString(error);
		return false;
	}

	return true;
}

bool renderHub::addScene()
{
	sceneList.push_back(scene());

	return true;
}

bool renderHub::deleteScene()
{
	return false;
}

bool renderHub::setSceneParameters()
{
	return false;
}

scene* renderHub::getScene(const int index)
{
	std::list<scene>::iterator iter = sceneList.begin();
	for( int i=0; i < index; ++i) {++iter;}
	return &(*iter);
}

void renderHub::setActiveScene(const int index)
{
	std::list<scene>::iterator iter = sceneList.begin();
	for( int i=0; i < index; ++i) {++iter;}
	activeScene = &(*iter);
}

void renderHub::run()
{
	/*	
	/	Just for testing and debug purposes I am ignoring the event-queue concept I want to take up later
	/	and manually add entities to the active scene
	*/
	if(!(activeScene->createStaticSceneObject(0,glm::vec3(0.0,0.0,0.0),glm::quat(),"../resources/materials/demoMaterial.slmtl",1)))
	{
		std::cout<<"Failed to create scene object"
				<<"\n";
	}
	if(!(activeScene->createStaticSceneObject(1,glm::vec3(0.0,0.0,-2.0),glm::quat(),"../resources/materials/demoMaterial.slmtl",1)))
	{
		std::cout<<"Failed to create scene object"
				<<"\n";
	}
	if(!(activeScene->createStaticSceneObject(2,glm::vec3(-2.0,0.0,0.0),glm::quat(),"../resources/materials/demoMaterial.slmtl",1)))
	{
		std::cout<<"Failed to create scene object"
				<<"\n";
	}
	//for(int i=2;i<10000;i++)
	//{
	//if(!(activeScene->createStaticSceneObject(i,glm::vec3(-i*2.0,-i*1.5,-i*2.0),glm::quat(),"../resources/materials/demoMaterial.slmtl",1)))
	//{
	//	std::cout<<"Failed to create scene object"
	//			<<"\n";
	//}
	//}


	if(!(activeScene->createSceneCamera(0,glm::vec3(1.0,2.0,1.0),glm::quat(),16.0f/9.0f,55.0f)))
	{
		std::cout<<"Failed to create camera"
				<<"\n";
	}
	if(!(activeScene->createSceneLight(0,glm::vec3(0.0,2.0,0.0),glm::vec4(1.0,1.0,1.0,1.0))))
	{
		std::cout<<"Failed to create light"
				<<"\n";
	}

	activeScene->setActiveCamera(0);
	
	activeScene->testing();

	framebufferObject testFBO(1200,675,true,false);
	testFBO.createColorAttachment(GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE);
	postProcessor pP;
	pP.init();

	running = true;
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	while(running && glfwGetWindowParam(GLFW_OPENED))
	{
		testFBO.bind();
		glViewport(0,0,1200,675);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		activeScene->render();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0,0,1200,675);
		pP.applyFxaa(&testFBO);

		glfwSwapBuffers();
		glfwSleep(0.01);
	}
}

void renderHub::runPoissonImageEditing()
{
	/*
	/	This is all just experimental stuff
	*/
	running = true;
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	/*
	/	Create framebuffers to work with.
	*/
	framebufferObject mainFbo(400,400,true,false);
	mainFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	framebufferObject fakePreviousFbo(400,400,true,false);
	fakePreviousFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	framebufferObject distanceMap(400,400,false,false);
	distanceMap.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	distanceMap.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	/*
	/	Load textures to work with.
	*/
	GLuint ftle;
	glGenTextures(1, &ftle);
	glBindTexture(GL_TEXTURE_2D, ftle);
	glfwLoadTexture2D("../resources/textures/fault_tolerant_vis/ftle.tga",0);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D,0);
	GLuint ftle_mask;
	glGenTextures(1, &ftle_mask);
	glBindTexture(GL_TEXTURE_2D, ftle_mask);
	glfwLoadTexture2D("../resources/textures/fault_tolerant_vis/ftle_mask.tga",0);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D,0);

	/*
	/	Create and initialize the post-processer
	*/
	postProcessor pP(400,400);
	if(!pP.init())
	{
		std::cout<<"Failed to create post processor"
				<<"\n";
	}

	/*
	/	Create the test bench
	*/
	ftvTestbench testBench;
	testBench.loadImageSequence();

	/*
	/	Test the new shader for creating a mask and a distance map at the same time.
	*/
	//GLfloat inpaintingRegions_data[] = {0.6f,0.6f,1.0f,1.0f};
	//GLuint inpaintingRegions;
	//glGenTextures(1, &inpaintingRegions);
	//glBindTexture(GL_TEXTURE_1D, inpaintingRegions);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA32F,1,0,GL_RGBA,GL_FLOAT,inpaintingRegions_data);
	//glBindTexture(GL_TEXTURE_1D,0);
	//
	//distanceMap.bind();
	//glViewport(0,0,distanceMap.getWidth(),distanceMap.getHeight());
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//pP.generateFtvMask(inpaintingRegions,1.0f);


	
	/*
	/	Copy image data to the framebuffer object.
	*/
	//mainFbo.bind();
	//glViewport(0,0,mainFbo.getWidth(),mainFbo.getHeight());
	//glEnable(GL_DEPTH);
	////glEnable(GL_BLEND);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	////pP.imageToFBO(ftle_mask);
	//pP.applyMaskToImageToFBO(ftle,ftle_mask,400,400);
	//fakePreviousFbo.bind();
	//glViewport(0,0,fakePreviousFbo.getWidth(),fakePreviousFbo.getHeight());
	//glEnable(GL_DEPTH);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//pP.imageToFBO(ftle);
	//distanceMap.bind();
	//glViewport(0,0,distanceMap.getWidth(),distanceMap.getHeight());
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//pP.generateDistanceMap(ftle_mask,400,400);

	/*
	/	Render Loop.
	*/
	//pP.applyImageInpainting(&mainFbo, ftle_mask, 1);
	while(running && glfwGetWindowParam(GLFW_OPENED))
	{
		//pP.applyPoisson(&mainFbo, &fakePreviousFbo, 5, ftle_mask, &distanceMap);
		//pP.applyImageInpainting(&mainFbo, ftle_mask, 1);

		mainFbo.bind();
		glViewport(0,0,mainFbo.getWidth(),mainFbo.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		testBench.getFrameConfigB(&distanceMap,&mainFbo);

		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport(0,0,700,700);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pP.FBOToFBO(&mainFbo);
		glfwSwapBuffers();
		glfwSleep(0.1);
	}
}