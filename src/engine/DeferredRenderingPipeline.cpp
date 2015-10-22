#include "DeferredRenderingPipeline.hpp"

DeferredRenderingPipeline::DeferredRenderingPipeline(EntityManager* entity_mngr,
										TransformComponentManager* transform_mngr,
										CameraComponentManager* camera_mngr,
										LightComponentManager* light_mngr)
	: m_lights_prepass(),
		m_geometry_pass(),
		m_shadow_map_pass(),
		m_active_camera(entity_mngr->create()), m_entity_mngr(entity_mngr), m_transform_mngr(transform_mngr), m_camera_mngr(camera_mngr), m_light_mngr(light_mngr)
{
	transform_mngr->addComponent(m_active_camera,Vec3(0.0f,0.0f,50.0f),Quat(),Vec3(1.0f));
	camera_mngr->addComponent(m_active_camera,0.01f,10000.0f);
}

DeferredRenderingPipeline::~DeferredRenderingPipeline()
{
}

void DeferredRenderingPipeline::geometryPass()
{
	RenderJobManager::RootNode m_root = m_geometry_pass.getRoot();

	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	Mat4x4 proj_matrix = m_camera_mngr->getProjectionMatrix( m_camera_mngr->getIndex(m_active_camera) );

	for(auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();
		shader.shader_prgm->setUniform("projection_matrix", proj_matrix);
		shader.shader_prgm->setUniform("view_matrix", view_matrix);

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

void DeferredRenderingPipeline::lightingPass()
{
	m_dfr_lighting_prgm->use();

	//TODO bind textures from framebuffer
	m_dfr_lighting_prgm->setUniform("albedoRGB_depth_tx2D",0);
	m_dfr_lighting_prgm->setUniform("normal_tx2D",1);
	m_dfr_lighting_prgm->setUniform("albedoRGB_tx2D",2);
	m_dfr_lighting_prgm->setUniform("specularRGB_roughness_tx2D",3);


	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	m_dfr_lighting_prgm->setUniform("view_matrix", view_matrix);

	int light_counter = 0;
	Vec3 light_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex( m_active_lightsources.front() ) );
	Vec3 light_intensity = m_light_mngr->getColour( m_light_mngr->getIndex( m_active_lightsources.front() ))
							 *m_light_mngr->getIntensity( m_light_mngr->getIndex( m_active_lightsources.front() ));

	m_dfr_lighting_prgm->setUniform("lights.position", light_position);
	m_dfr_lighting_prgm->setUniform("lights.intensity", light_intensity);

	//shader.shader_prgm->setUniform("lights.position", glm::vec3(2500.0,2500.0,1500.0));
	//shader.shader_prgm->setUniform("lights.intensity", glm::vec3(50000.0));
	m_dfr_lighting_prgm->setUniform("num_lights", light_counter);

	m_dfr_fullscreenQuad->draw();
}

void DeferredRenderingPipeline::run()
{
	std::cout<<"----------------------------\n"
			<<"SPACE LION - Early Prototype\n"
			<<"----------------------------\n";
	// Initialize GLFW
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

	// Register callback functions
	glfwSetWindowSizeCallback(m_active_window,windowSizeCallback);
	glfwSetWindowCloseCallback(m_active_window,windowCloseCallback);

	Controls::init(m_transform_mngr,&m_active_camera);
	Controls::setControlCallbacks(m_active_window);

	// Initialize glew
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
	// Apparently glweInit() causes a GL ERROR 1280, so let's just catch that...
	glGetError();

	// Create dumy geometry for deferred rendering
	std::vector<Vertex_pu> vertex_array = {{ Vertex_pu(-1.0,-1.0f,-1.0f,0.0f,0.0f),
											Vertex_pu(-1.0,1.0f,-1.0f,0.0f,1.0f),
											Vertex_pu(1.0,1.0f,-1.0f,1.0f,1.0f),
											Vertex_pu(1.0,-1.0f,-1.0f,1.0f,0.0f) }};
	std::vector<GLuint> index_array = {{ 0,2,1,2,0,3 }};

	m_dfr_fullscreenQuad = m_resource_mngr.createMesh("fullscreen_quad",vertex_array,index_array,GL_TRIANGLES);

	// Load lighting shader program for deferred rendering
	m_dfr_lighting_prgm = m_resource_mngr.createShaderProgram({"../resources/shaders/genericPostProc_v.glsl","../resources/shaders/dfr_lighting_f.glsl"});

	std::cout<<m_dfr_lighting_prgm->getLog()<<std::endl;

	// TODO: Implement actual rendering pipeline here
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	// Create G-Buffer
	FramebufferObject gBuffer(1600,900,true);
	gBuffer.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	gBuffer.createColorAttachment(GL_RGB32F,GL_RGB,GL_FLOAT);
	gBuffer.createColorAttachment(GL_RGB8,GL_RGB,GL_UNSIGNED_BYTE);
	gBuffer.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	std::cout<<gBuffer.getLog()<<std::endl;

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

		//TODO geometry pass
		gBuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		height = gBuffer.getHeight();
		width = gBuffer.getWidth();
		glViewport(0, 0, width, height);

		geometryPass();
	
		//TODO lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwGetFramebufferSize(m_active_window, &width, &height);
		glViewport(0, 0, width, height);
		
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		gBuffer.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		gBuffer.bindColorbuffer(1);
		glActiveTexture(GL_TEXTURE2);
		gBuffer.bindColorbuffer(2);
		glActiveTexture(GL_TEXTURE3);
		gBuffer.bindColorbuffer(3);

		lightingPass();

		//TODO post processing

		glfwSwapBuffers(m_active_window);
		glfwPollEvents();
	}

	/* Clear resources while context is still alive */
	m_geometry_pass.clearRenderJobs();
	m_resource_mngr.clearLists();
	m_dfr_fullscreenQuad.reset();
	m_dfr_lighting_prgm.reset();

	glfwMakeContextCurrent(NULL);
}

void DeferredRenderingPipeline::requestRenderJob(Entity entity, std::string material_path, std::string mesh_path, bool cast_shadow)
{
	m_jobRequest_queue.push(RenderJobRequest(entity,material_path,mesh_path));
}

void DeferredRenderingPipeline::addLightsource(Entity entity)
{
	m_active_lightsources.push_back(entity);
}

void DeferredRenderingPipeline::processRenderJobRequest()
{
	while(!m_jobRequest_queue.empty())
	{
		RenderJobRequest new_jobRequest = m_jobRequest_queue.pop();

		std::shared_ptr<Material> material = m_resource_mngr.createMaterial(new_jobRequest.material_path);
		std::shared_ptr<Mesh> mesh = m_resource_mngr.createMesh(new_jobRequest.mesh_path);

		m_geometry_pass.addRenderJob(RenderJob(new_jobRequest.entity,material,mesh));
	}
}

void DeferredRenderingPipeline::windowSizeCallback(GLFWwindow* window, int width, int height)
{

}

void DeferredRenderingPipeline::windowCloseCallback(GLFWwindow* window)
{

}