#include "RenderingPipeline.hpp"

RenderingPipeline::RenderingPipeline(EntityManager* entity_mngr,
										TransformComponentManager* transform_mngr,
										CameraComponentManager* camera_mngr,
										LightComponentManager* light_mngr)
	: m_lights_prepass(entity_mngr,transform_mngr,camera_mngr,light_mngr),
		m_forward_render_pass(entity_mngr,transform_mngr,camera_mngr,light_mngr),
		m_shadow_map_pass(entity_mngr,transform_mngr,camera_mngr,light_mngr),
		m_active_camera(entity_mngr->create()), m_entity_mngr(entity_mngr), m_transform_mngr(transform_mngr)
{
	transform_mngr->addComponent(m_active_camera,Vec3(0.0f,0.0f,50.0f),Quat(),Vec3(1.0f));
	camera_mngr->addComponent(m_active_camera,0.01f,10000.0f);
}

RenderingPipeline::~RenderingPipeline()
{
}

void RenderingPipeline::run()
{
	std::cout<<"----------------------------\n"
			<<"SPACE LION - Early Prototype\n"
			<<"----------------------------\n";
	//	Initialize GLFW
	if(!glfwInit())
	{
		std::cout<<"-----\n"
				<<"The time is out of joint - O cursed spite,\n"
				<<"That ever I was born to set it right!\n"
				<<"-----\n"
				<<"Error: Couldn't initialize glfw.";
	}

	m_active_window = glfwCreateWindow(1600,900,"Space-Lion",NULL,NULL);

	if(!m_active_window)
	{
		std::cout<<"-----\n"
				<<"The time is out of joint - O cursed spite,\n"
				<<"That ever I was born to set it right!\n"
				<<"-----\n"
				<<"Error: Couldn't open glfw window";

		glfwTerminate();
	}

	glfwMakeContextCurrent(m_active_window);

	/* Register callback functions */
	glfwSetWindowSizeCallback(m_active_window,windowSizeCallback);
	glfwSetWindowCloseCallback(m_active_window,windowCloseCallback);

	Controls::init(m_transform_mngr,&m_active_camera);
	Controls::setControlCallbacks(m_active_window);

	/*	Initialize glew */
	//glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if( GLEW_OK != error)
	{
		std::cout<<"-----\n"
				<<"The time is out of joint - O cursed spite,\n"
				<<"That ever I was born to set it right!\n"
				<<"-----\n"
				<<"Error: "<<glewGetErrorString(error);
	}
	/* Apparently glweInit() causes a GL ERROR 1280, so let's just catch that... */
	glGetError();

	// TODO: Implement actual rendering pipeline here
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	/* Hardcoded tests */
	//	Entity debug_cube = m_entity_mngr->create();
	//	m_transform_mngr->addComponent(debug_cube,Vec3(0.0),Quat(),Vec3(1.0));
	//	std::shared_ptr<Mesh> geomPtr;
	//	std::shared_ptr<Material> matPtr;
	//	geomPtr = m_resource_mngr.createBox();
	//	matPtr = m_resource_mngr.createMaterial("../resources/materials/debug.slmtl");
	//	m_forward_render_pass.addRenderJob(RenderJob(debug_cube,matPtr,geomPtr));

	//	std::cout<<"Camera id: "<<m_active_camera.id()<<std::endl;
	//	std::cout<<"Debug cube id: "<<debug_cube.id()<<std::endl;

	double t0,t1 = 0.0;

	while (!glfwWindowShouldClose(m_active_window))
	{
		t0 = t1;
		t1 = glfwGetTime();
		double dt = t1 - t0;

		//std::cout<<"Timestep: "<<dt<<std::endl;

		Controls::checkKeyStatus(m_active_window,dt);

		/* Process new RenderJobRequest */
		processRenderJobRequest();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		glfwGetFramebufferSize(m_active_window, &width, &height);
		glViewport(0, 0, width, height);

		m_forward_render_pass.processRenderJobs(m_active_camera,m_active_lightsources);

		glfwSwapBuffers(m_active_window);
		glfwPollEvents();
	}

	/* Clear resources while context is still alive */
	m_forward_render_pass.clearRenderJobs();
	m_resource_mngr.clearLists();

	glfwMakeContextCurrent(NULL);
}

void RenderingPipeline::requestRenderJob(Entity entity, std::string material_path, std::string mesh_path, bool cast_shadow)
{
	m_jobRequest_queue.push(RenderJobRequest(entity,material_path,mesh_path));
}

void RenderingPipeline::addLightsource(Entity entity)
{
	m_active_lightsources.push_back(entity);
}

void RenderingPipeline::processRenderJobRequest()
{
	while(!m_jobRequest_queue.empty())
	{
		RenderJobRequest new_jobRequest = m_jobRequest_queue.pop();

		std::shared_ptr<Material> material = m_resource_mngr.createMaterial(new_jobRequest.material_path);
		std::shared_ptr<Mesh> mesh = m_resource_mngr.createMesh(new_jobRequest.mesh_path);

		m_forward_render_pass.addRenderJob(RenderJob(new_jobRequest.entity,material,mesh));
	}
}

void RenderingPipeline::windowSizeCallback(GLFWwindow* window, int width, int height)
{

}

void RenderingPipeline::windowCloseCallback(GLFWwindow* window)
{

}