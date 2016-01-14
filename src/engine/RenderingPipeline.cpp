#include "RenderingPipeline.hpp"

RenderingPipeline::RenderingPipeline(EntityManager* entity_mngr,
										TransformComponentManager* transform_mngr,
										CameraComponentManager* camera_mngr,
										PointlightComponentManager* light_mngr)
	: m_lights_prepass(),
		m_forward_render_pass(),
		m_shadow_map_pass(),
		m_active_camera(entity_mngr->create()), m_entity_mngr(entity_mngr), m_transform_mngr(transform_mngr), m_camera_mngr(camera_mngr), m_light_mngr(light_mngr)
{
	transform_mngr->addComponent(m_active_camera,Vec3(0.0f,0.0f,50.0f),Quat(),Vec3(1.0f));
	camera_mngr->addComponent(m_active_camera,0.01f,10000.0f);
}

RenderingPipeline::~RenderingPipeline()
{
}

void RenderingPipeline::forwardPass()
{
	RenderJobManager::RootNode m_root = m_forward_render_pass.getRoot();

	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	Mat4x4 proj_matrix = m_camera_mngr->getProjectionMatrix( m_camera_mngr->getIndex(m_active_camera) );

	for(auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();
		shader.shader_prgm->setUniform("projection_matrix", proj_matrix);
		shader.shader_prgm->setUniform("view_matrix", view_matrix);

		int light_counter = 0;

		Vec3 light_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex( m_active_lightsources.front() ) );
		Vec3 light_intensity = m_light_mngr->getColour( m_light_mngr->getIndex( m_active_lightsources.front() ))
								 *m_light_mngr->getLumen( m_light_mngr->getIndex( m_active_lightsources.front() ));

		shader.shader_prgm->setUniform("lights.position", light_position);
		shader.shader_prgm->setUniform("lights.intensity", light_intensity);

		//shader.shader_prgm->setUniform("lights.position", glm::vec3(2500.0,2500.0,1500.0));
		//shader.shader_prgm->setUniform("lights.intensity", glm::vec3(50000.0));
		shader.shader_prgm->setUniform("num_lights", light_counter);

		for(auto& material : shader.materials)
		{
			material.material->use();

			for(auto& mesh : material.meshes)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;

				for(auto& entity : mesh.enities)
				{
					uint transform_index = m_transform_mngr->getIndex(entity);
					Mat4x4 model_matrix = m_transform_mngr->getWorldTransformation(transform_index);

					//	std::cout<<"=========================="<<std::endl;
					//	std::cout<<model_matrix[0][0]<<" "<<model_matrix[1][0]<<" "<<model_matrix[2][0]<<" "<<model_matrix[3][0]<<std::endl;
					//	std::cout<<model_matrix[0][1]<<" "<<model_matrix[1][1]<<" "<<model_matrix[2][1]<<" "<<model_matrix[3][1]<<std::endl;
					//	std::cout<<model_matrix[0][2]<<" "<<model_matrix[1][2]<<" "<<model_matrix[2][2]<<" "<<model_matrix[3][2]<<std::endl;
					//	std::cout<<model_matrix[0][3]<<" "<<model_matrix[1][2]<<" "<<model_matrix[2][3]<<" "<<model_matrix[3][3]<<std::endl;
					//	std::cout<<"=========================="<<std::endl;

					Mat4x4 model_view_matrix = view_matrix * model_matrix;
					//	Mat4x4 model_view_matrix = view_matrix;
					std::string uniform_name("model_view_matrix[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(uniform_name.c_str(), model_view_matrix);

					instance_counter++;

					if(instance_counter == 128)
					{
						mesh.mesh->draw(instance_counter);
						instance_counter = 0;
					}
				}
				mesh.mesh->draw(instance_counter);
				instance_counter = 0;
			}
		}
	}
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

		forwardPass();

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