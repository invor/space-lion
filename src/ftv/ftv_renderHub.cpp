#include "ftv_renderHub.h"


Ftv_RenderHub::Ftv_RenderHub(void)
{
}

Ftv_RenderHub::~Ftv_RenderHub(void)
{
}

void Ftv_RenderHub::runFtvVolumeTest()
{
	glfwMakeContextCurrent(activeWindow);

	Ftv_Scene tScene;
	std::shared_ptr<Mesh> geomPtr;
	std::shared_ptr<Texture3D> volPtr;
	std::shared_ptr<Texture3D> maskPtr;
	std::shared_ptr<GLSLProgram> prgmPtr;

	ftv_resourceMngr.createBox(geomPtr);

	FtvTestbench testbench;
	float* vol_mask = new float[67 * 67 * 67];
	testbench.createVolumeMask(vol_mask, glm::ivec3(67, 67, 67), glm::ivec3(15, 15, 15), glm::ivec3(35, 35, 35));
	ftv_resourceMngr.createTexture3D(vol_mask, glm::ivec3(67, 67, 67), GL_RED, GL_RED, maskPtr);
	delete [] vol_mask;
	maskPtr->texParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	maskPtr->texParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	ftv_resourceMngr.createTexture3D("../resources/volumeData/f.raw",glm::ivec3(67,67,67),volPtr);
	ftv_resourceMngr.createFtvShaderProgram(FTV_VOLUME_RAYCASTING,prgmPtr);


	tScene.createFtvVolumetricSceneObject(0, glm::vec3(0.0, 0.0, 0.0), glm::quat(), glm::vec3(1.0, 1.0, 1.0), geomPtr, volPtr, maskPtr, prgmPtr);

	//if(!(tScene.createVolumetricSceneObject(0,glm::vec3(0.0,0.0,0.0),glm::quat(),glm::vec3(1.0,1.0,1.0),geomPtr,volPtr,prgmPtr)))
	//{
	//	std::cout<<"Failed to create scene object"
	//			<<"\n";
	//}

	if(!(tScene.createSceneCamera(0,glm::vec3(0.0,0.0,1.5),glm::quat(),16.0f/9.0f,55.0f)))
	{
		std::cout<<"Failed to create camera"
				<<"\n";
	}

	tScene.setActiveCamera(0);

	running = true;
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);

	while (running && !glfwWindowShouldClose(activeWindow))
	{
		Controls::updateCamera(activeWindow, tScene.getActiveCamera());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		tScene.ftvRenderVolumetricObjects();

		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}
}

void Ftv_RenderHub::runFtv()
{
	glfwMakeContextCurrent(activeWindow);

	/*
	/	This is all just experimental stuff
	*/
	running = true;
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	/*
	/	Create framebuffers to work with.
	*/
	FramebufferObject primaryFbo(400,400,true,false);
	primaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject secondaryFbo(400,400,true,false);
	secondaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject maskFbo(400,400,false,false);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	/*
	/	Create and initialize the post-processer
	*/
	Ftv_PostProcessor pP(400,400);
	if(!pP.ftv_init(&ftv_resourceMngr))
	{
		std::cout<<"Failed to create post processor"
				<<"\n";
	}

	/*
	/	Create the test bench
	*/
	FtvTestbench testBench;
	testBench.loadImageSequence();
	testBench.initMasks();

	/*
	/	Render Loop.
	*/
	
	testBench.getFrameConfigC(&maskFbo,&secondaryFbo);

	while (running && !glfwWindowShouldClose(activeWindow))
	{
		/*
		/	Alternate between secondary and primary fbo.
		/	Begin with primary
		*/
		testBench.getFrameConfigC(&maskFbo,&primaryFbo);
		
		//pP.applyPoisson(&primaryFbo, &secondaryFbo, 20, &maskFbo);
		pP.applyImageInpainting(&primaryFbo, &maskFbo, 200);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		pP.FBOToFBO(&primaryFbo);
		glfwSwapBuffers(activeWindow);
		glfwPollEvents();

		/*
		/	Switch to secondary
		*/
		testBench.getFrameConfigC(&maskFbo,&secondaryFbo);
		
		//pP.applyPoisson(&secondaryFbo, &primaryFbo, 20, &maskFbo);
		pP.applyImageInpainting(&secondaryFbo, &maskFbo, 200);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		pP.FBOToFBO(&secondaryFbo);
		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}
}

void Ftv_RenderHub::runInpaintingTest()
{
	glfwMakeContextCurrent(activeWindow);

	running = true;

	/*	Create framebuffers to work with. */
	FramebufferObject primaryFbo(400,400,true,false);
	primaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject secondaryFbo(400,400,true,false);
	secondaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject maskFbo(400,400,false,false);
	maskFbo.createColorAttachment(GL_RG32F,GL_RG,GL_FLOAT);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	/*	Create and initialize the post-processer. */
	Ftv_PostProcessor pP(400,400);
	if(!pP.ftv_init(&ftv_resourceMngr))
		std::cout<<"Failed to create post processor" <<"\n";

	/*	Create the test bench. */
	FtvTestbench testBench;
	testBench.loadImageSequence();
	testBench.initMasks();
	
	testBench.getFrameConfigC(&maskFbo,&secondaryFbo);
	testBench.getFrameConfigC(&maskFbo,&primaryFbo);
	testBench.getFrameConfigC(&maskFbo,&primaryFbo);
	testBench.getFrameConfigC(&maskFbo,&primaryFbo);
	testBench.getFrameConfigC(&maskFbo,&primaryFbo);


	/*	Some additional FBOs are required for coherence computations */
	glm::ivec2 imgDim = glm::ivec2(400,400);
	FramebufferObject gaussianFbo(imgDim.x,imgDim.y,false,false);
	gaussianFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject gradientFbo(imgDim.x,imgDim.y,false,false);
	gradientFbo.createColorAttachment(GL_RG32F,GL_RG,GL_FLOAT);
	FramebufferObject hesseFbo(imgDim.x,imgDim.y,false,false);
	hesseFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject coherenceFbo(imgDim.x,imgDim.y,false,false);
	coherenceFbo.createColorAttachment(GL_RGB32F,GL_RGB,GL_FLOAT);

	/*	Compute the coherence flow field and strength */
	pP.applyFtvGaussian(&primaryFbo,&gaussianFbo,&maskFbo,1.4f,3);
	pP.computeStructureTensor(&gaussianFbo,&hesseFbo);
	//pP.computeGradient(&gaussianFbo,&gradientFbo);
	//pP.applyFtvGaussian(&gradientFbo,&gradientFbo,&maskFbo,1.4f,3);
	//pP.computeHesse(&gradientFbo,&hesseFbo);
	pP.applyFtvGaussian(&hesseFbo,&hesseFbo,&maskFbo,4.0f,6);
	pP.computeCoherence(&hesseFbo,&coherenceFbo);

	while (running && !glfwWindowShouldClose(activeWindow))
	{
		#if TIMER
			double start = glfwGetTime();
		#endif	

		//pP.applyImprovedImageInpainting(&primaryFbo,&maskFbo,25);
		//pP.applyImageInpainting(&primaryFbo,&maskFbo,25);
		pP.applyPoisson(&primaryFbo,&secondaryFbo,&maskFbo,20,1);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		pP.FBOToFBO(&primaryFbo);
		glfwSwapBuffers(activeWindow);
		glfwPollEvents();

		#if TIMER
			double end = glfwGetTime();
			std::cout<<end-start<<std::endl;
		#endif	
	}
}

void Ftv_RenderHub::runTextureAdvectionTest()
{
	glfwMakeContextCurrent(activeWindow);

	running = true;
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	/*	Create framebuffers to work with. */
	FramebufferObject primaryFbo(400,400,true,false);
	primaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject secondaryFbo(400,400,true,false);
	secondaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject maskFbo(400,400,false,false);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	/*	Create and initialize the post-processer. */
	Ftv_PostProcessor pP(400,400);
	if(!pP.ftv_init(&ftv_resourceMngr))
		std::cout<<"Failed to create post processor"<<"\n";

	/*	Create the test bench. */
	FtvTestbench testbench;
	if(!testbench.loadVectorFieldSequenceTo3D(100,150)) std::cout<<"Couldn't load vector field data.\n";
	testbench.loadImageSequence();
	testbench.initMasks();

	#if TIMER
			double start = glfwGetTime();
	#endif

	/*	Load vector field */
	GLuint vecFieldTx;
	testbench.getVectorTexture3D(vecFieldTx);

	/* Get first frame */
	testbench.getFrameConfigC(&maskFbo,&primaryFbo);
	
	/* Advect the first frame */
	secondaryFbo.bind();
	pP.textureAdvection(&primaryFbo,vecFieldTx);

	/*	Get the next frame */
	testbench.getFrameConfigC(&maskFbo,&primaryFbo);

	/*	Combine the current frame and the advected previous frame with poisson */
	pP.applyPoisson(&primaryFbo,&secondaryFbo,&maskFbo,600,0);

	#if TIMER
			double end = glfwGetTime();
			std::cout<<end-start<<std::endl;
	#endif

	/*	Render Loop. */
	while(running && !glfwWindowShouldClose(activeWindow))
	{
		//pP.applyPoisson(&primaryFbo,&secondaryFbo,&maskFbo,75,0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		pP.FBOToFBO(&primaryFbo);

		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}
}

void Ftv_RenderHub::runFtvGuidanceFieldTest()
{
	glfwMakeContextCurrent(activeWindow);

	running = true;
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	/*	Create framebuffers to work with */
	FramebufferObject primaryFbo(400,400,true,false);
	primaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject secondaryFbo(400,400,true,false);
	secondaryFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	FramebufferObject maskFbo(400,400,false,false);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	maskFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	/*	Create and initialize the post-processer */
	Ftv_PostProcessor pP(400,400);
	if(!pP.ftv_init(&ftv_resourceMngr))
	{
		std::cout<<"Failed to create post processor for guidance field test."
				<<"\n";
	}

	/*	Create the test bench */
	FtvTestbench testbench;
	testbench.loadImageSequence();
	if(!testbench.loadVectorFieldSequence()) std::cout<<"Couldn't load vector field data.\n";
	testbench.initMasks();

	GLuint vecFieldTx;
	int index = 3;
	//testbench.getVectorTexture(vecFieldTx,1);

	testbench.getFrameConfigC(&maskFbo,&primaryFbo);
	testbench.getFrameConfigC(&maskFbo,&primaryFbo);
	testbench.getFrameConfigC(&maskFbo,&primaryFbo);
	testbench.getFrameConfigC(&maskFbo,&primaryFbo);

	/*	Render Loop */
	while(running && !glfwWindowShouldClose(activeWindow))
	{
		testbench.getVectorTexture(vecFieldTx,(index%51));
		//primaryFbo.bind();
		//pP.imageToFBO(vecFieldTx);
		//index++;

		//testbench.getFrameConfigC(&maskFbo,&primaryFbo);
		//pP.applyGuidedPoisson(&primaryFbo,vecFieldTx,&maskFbo,1);
		//pP.applyGuidedImageInpainting(&primaryFbo, &maskFbo,vecFieldTx,32);
		pP.applyLicInpainting(&primaryFbo, vecFieldTx, &maskFbo, 50);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(activeWindow, &width, &height);
		glViewport(0, 0, width, height);

		pP.FBOToFBO(&primaryFbo);
		glfwSwapBuffers(activeWindow);
		glfwPollEvents();
	}
}