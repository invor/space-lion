#include "DeferredRenderingPipeline.hpp"

DeferredRenderingPipeline::DeferredRenderingPipeline(EntityManager* entity_mngr,
										ResourceManager* resource_mngr,
										TransformComponentManager* transform_mngr,
										CameraComponentManager* camera_mngr,
										PointlightComponentManager* light_mngr,
										SunlightComponentManager* sunligh_mngr,
										AtmosphereComponentManager* atmosphere_mngr,
										StaticMeshComponentManager* staticMesh_mngr)
	: m_lights_prepass(),
		m_staticMeshes_pass(),
		m_shadow_map_pass(),
		m_active_camera(entity_mngr->create()),
		m_entity_mngr(entity_mngr),
		m_resource_mngr(resource_mngr),
		m_transform_mngr(transform_mngr),
		m_camera_mngr(camera_mngr),
		m_light_mngr(light_mngr),
		m_sunlight_mngr(sunligh_mngr),
		m_atmosphere_mngr(atmosphere_mngr),
		m_staticMesh_mngr(staticMesh_mngr)
{
	transform_mngr->addComponent(m_active_camera,Vec3(0.0f,0.0f,20.0f),Quat(),Vec3(1.0f));
	camera_mngr->addComponent(m_active_camera, 0.1f, 15000.0f);
}

DeferredRenderingPipeline::~DeferredRenderingPipeline()
{
}

void DeferredRenderingPipeline::orderIndependentTransparencyPass()
{
}

void DeferredRenderingPipeline::geometryPass()
{
	RenderJobManager::RootNode m_root = m_staticMeshes_pass.getRoot();

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

void DeferredRenderingPipeline::atmospherePass()
{
	AtmosphereComponentManager::Data const * const atmosphere_data = m_atmosphere_mngr->getData();
	uint num_entities = atmosphere_data->used;

	if( num_entities == 0 ) return;

	glDisable(GL_CULL_FACE);

	// TODO: Try to remove redundancy of getting this information twice during rendering a single frame
	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	Mat4x4 proj_matrix = m_camera_mngr->getProjectionMatrix( m_camera_mngr->getIndex(m_active_camera) );

	auto shader_prgm = atmosphere_data->material[0]->getShaderProgram();

	shader_prgm->use();
	shader_prgm->setUniform("normal_depth_tx2D",0);
	shader_prgm->setUniform("projection_matrix", proj_matrix);
	shader_prgm->setUniform("view_matrix", view_matrix);
	shader_prgm->setUniform("camera_position", m_transform_mngr->getPosition( m_transform_mngr->getIndex(m_active_camera) ));

	Vec3 camera_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex(m_active_camera));
	Vec3 atmosphere_center_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex(atmosphere_data->entity[0]));

	int sun_count = m_sunlight_mngr->getData()->used;
	for(int i=0; i<sun_count; i++)
	{
		std::string sun_direction_uniform("suns[" + std::to_string(i) + "].sun_direction");
		std::string sun_luminance_uniform("suns[" + std::to_string(i) + "].sun_luminance");

		Vec3 light_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex( m_sunlight_mngr->getData()->entity[i] ) );
		shader_prgm->setUniform(sun_direction_uniform.c_str(), (light_position-camera_position) );

		// Compute luminance of sun (simplified to a pointlight) just before it hits the atmosphere
		float sun_luminance = m_sunlight_mngr->getLumen(i);
		float distance = std::sqrt( 
			(light_position-atmosphere_center_position).x*(light_position-atmosphere_center_position).x +
			(light_position-atmosphere_center_position).y*(light_position-atmosphere_center_position).y +
			(light_position-atmosphere_center_position).z*(light_position-atmosphere_center_position).z ) - atmosphere_data->max_altitude[0];
		sun_luminance = sun_luminance/(4.0f * 3.14f * std::pow( distance ,2.0f) );

		shader_prgm->setUniform(sun_luminance_uniform.c_str(), sun_luminance);
		//std::cout<<"Luminance: "<<sun_luminance<<std::endl;
		//std::cout<<"Distance: "<< distance <<std::endl;
	}
	shader_prgm->setUniform("sun_count", sun_count);

	/*	Draw all entities instanced */
	int instance_counter = 0;
	std::string atmosphere_center_uniform;
	std::string max_altitude_uniform;
	std::string min_altitude_uniform;
	std::string model_uniform;

	for(uint i=0; i<num_entities; i++)
	{
		uint transform_index = m_transform_mngr->getIndex(atmosphere_data->entity[i]);
		Mat4x4 model_matrix = m_transform_mngr->getWorldTransformation(transform_index);
		model_uniform = ("model_matrix[" + std::to_string(instance_counter) + "]");
		shader_prgm->setUniform(model_uniform.c_str(), model_matrix);

		max_altitude_uniform = ("max_altitude[" + std::to_string(instance_counter) + "]");
		shader_prgm->setUniform( max_altitude_uniform.c_str() , atmosphere_data->max_altitude[i] );

		min_altitude_uniform = ("min_altitude[" + std::to_string(instance_counter) + "]");
		shader_prgm->setUniform( min_altitude_uniform.c_str() , atmosphere_data->min_altitude[i] );

		atmosphere_center_uniform = ("atmosphere_center[" + std::to_string(instance_counter) + "]");
		shader_prgm->setUniform(atmosphere_center_uniform.c_str(),m_transform_mngr->getPosition(transform_index));
		
		glEnable(GL_TEXTURE_3D);
		glActiveTexture(GL_TEXTURE1);
		shader_prgm->setUniform("rayleigh_inscatter_tx3D",1);
		atmosphere_data->material[i]->getRayleighInscatterTable()->bindTexture();

		glActiveTexture(GL_TEXTURE2);
		shader_prgm->setUniform("mie_inscatter_tx3D",2);
		atmosphere_data->material[i]->getMieInscatterTable()->bindTexture();

		instance_counter++;

		if(instance_counter == 128)
		{
			m_atmosphere_boundingSphere->draw(instance_counter);
			instance_counter = 0;
		}
	}

	m_atmosphere_boundingSphere->draw(instance_counter);
}

void DeferredRenderingPipeline::lightingPass()
{
	m_lighting_prgm->use();

	// Bind textures from framebuffer
	m_lighting_prgm->setUniform("normal_depth_tx2D",0);
	m_lighting_prgm->setUniform("albedoRGB_tx2D",1);
	m_lighting_prgm->setUniform("specularRGB_roughness_tx2D",2);

	m_lighting_prgm->setUniform("atmosphereRGBA_tx2D",3);


	// Get information on active camera
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	float fovy = m_camera_mngr->getFovy(m_camera_mngr->getIndex(m_active_camera));
	float aspect_ratio = m_camera_mngr->getAspectRatio(m_camera_mngr->getIndex(m_active_camera));
	Vec2 aspect_fovy(aspect_ratio,fovy);
	float exposure = m_camera_mngr->getExposure(m_camera_mngr->getIndex(m_active_camera));
	m_lighting_prgm->setUniform("view_matrix", view_matrix);
	m_lighting_prgm->setUniform("aspect_fovy", aspect_fovy);
	m_lighting_prgm->setUniform("exposure", exposure);

	// Get information on scene light...this is basically a placeholder version
	uint light_counter = 0;
	for(auto light_entity : m_active_lightsources )
	{
		Vec3 light_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex( light_entity ) );
		// TODO check how to correctly mix light color and lumens
		Vec3 light_intensity = m_light_mngr->getColour( m_light_mngr->getIndex( light_entity ))
								 *m_light_mngr->getLumen( m_light_mngr->getIndex( light_entity ));

		std::string light_position_uniform_name("lights[" + std::to_string(light_counter) + "].position");
		std::string light_intensity_uniform_name("lights[" + std::to_string(light_counter) + "].lumen");

		m_lighting_prgm->setUniform(light_position_uniform_name.c_str(), light_position);
		m_lighting_prgm->setUniform(light_intensity_uniform_name.c_str(), light_intensity);

		light_counter++;
	}

	m_lighting_prgm->setUniform("num_lights", (int) m_active_lightsources.size() );

	//TODO add sunlight
	int sun_count = m_sunlight_mngr->getComponentCount();
	for(int i=0; i < sun_count; i++)
	{
		std::string sun_position_uniform("suns[" + std::to_string(i) + "].position");
		std::string sun_illuminance_uniform("suns[" + std::to_string(i) + "].illuminance");

		Vec3 light_position = m_transform_mngr->getPosition( m_transform_mngr->getIndex( m_sunlight_mngr->getData()->entity[i] ) );
		m_lighting_prgm->setUniform(sun_position_uniform.c_str(), light_position );

		// Compute luminance of sun (simplified to a pointlight) just before it hits the atmosphere
		float sun_luminous_power = m_sunlight_mngr->getLumen(i);
		
		//TODO compute actual illuminance

		m_lighting_prgm->setUniform(sun_illuminance_uniform.c_str(), 100000.0f );
	}

	m_lighting_prgm->setUniform("num_suns", sun_count);

	m_fullscreenQuad->draw();
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

	// Create dummy geometry for deferred rendering
	std::vector<Vertex_pu> vertex_array = {{ Vertex_pu(-1.0,-1.0f,-1.0f,0.0f,0.0f),
											Vertex_pu(-1.0,1.0f,-1.0f,0.0f,1.0f),
											Vertex_pu(1.0,1.0f,-1.0f,1.0f,1.0f),
											Vertex_pu(1.0,-1.0f,-1.0f,1.0f,0.0f) }};
	std::vector<GLuint> index_array = {{ 0,2,1,2,0,3 }};

	m_fullscreenQuad = m_resource_mngr->createMesh("fullscreen_quad",vertex_array,index_array,GL_TRIANGLES);

	// Load lighting shader program for deferred rendering
	m_lighting_prgm = m_resource_mngr->createShaderProgram({"../resources/shaders/genericPostProc_v.glsl","../resources/shaders/dfr_lighting_f.glsl"});

	// Create basic boundingbox for atmosphere rendering
	m_atmosphere_boundingSphere = m_resource_mngr->createIcoSphere(0);

	// Create G-Buffer
	FramebufferObject gBuffer(1600,900,true);
	gBuffer.createColorAttachment(GL_RGBA16F,GL_RGBA,GL_HALF_FLOAT);
	gBuffer.createColorAttachment(GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE);
	gBuffer.createColorAttachment(GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE);
	std::cout<<gBuffer.getLog()<<std::endl;

	FramebufferObject atmosphere_fbo(1600,900,true);
	atmosphere_fbo.createColorAttachment(GL_RGBA16F,GL_RGBA,GL_HALF_FLOAT);

	// Set some OpenGL states
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	double t0,t1 = 0.0;

	while (!glfwWindowShouldClose(m_active_window))
	{
		t0 = t1;
		t1 = glfwGetTime();
		double dt = t1 - t0;

		//std::cout<<"Timestep: "<<dt<<std::endl;

		Controls::checkKeyStatus(m_active_window,dt);

		// Process new RenderJobRequest
		//processRenderJobRequest();

		registerStaticMeshComponents();

		registerLightComponents();

		// Process new atmoshpere entities
		m_atmosphere_mngr->processNewComponents();

		// Geometry pass
		m_camera_mngr->setCameraAttributes(m_camera_mngr->getIndex(m_active_camera),0.01,10000.0);
		gBuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int width, height;
		height = gBuffer.getHeight();
		width = gBuffer.getWidth();
		glViewport(0, 0, width, height);

		geometryPass();

		// Atmosphere pass
		m_camera_mngr->setCameraAttributes(m_camera_mngr->getIndex(m_active_camera),100.0,100000000.0);

		atmosphere_fbo.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		height = atmosphere_fbo.getHeight();
		width = atmosphere_fbo.getWidth();
		glViewport(0, 0, width, height);

		glActiveTexture(GL_TEXTURE0);
		gBuffer.bindColorbuffer(0);
		atmospherePass();
	
		// Lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwGetFramebufferSize(m_active_window, &width, &height);
		glViewport(0, 0, width, height);
		
		glActiveTexture(GL_TEXTURE0);
		gBuffer.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		gBuffer.bindColorbuffer(1);
		glActiveTexture(GL_TEXTURE2);
		gBuffer.bindColorbuffer(2);

		glActiveTexture(GL_TEXTURE3);
		atmosphere_fbo.bindColorbuffer(0);
		
		lightingPass();

		//TODO post processing

		glfwSwapBuffers(m_active_window);
		glfwPollEvents();
	}

	/* Clear resources while context is still alive */
	m_staticMeshes_pass.clearRenderJobs();
	m_resource_mngr->clearLists();
	m_fullscreenQuad.reset();
	m_lighting_prgm.reset();
	m_atmosphere_boundingSphere.reset();

	glfwMakeContextCurrent(NULL);
}

void DeferredRenderingPipeline::requestRenderJob(const RenderJobRequest& new_request)
{
	m_renderJobRequest_queue.push(new_request);
}

void DeferredRenderingPipeline::addLightsource(Entity entity)
{
	m_active_lightsources.push_back(entity);
}

void DeferredRenderingPipeline::setActiveCamera(Entity entity)
{
	m_active_camera = entity;
}

void DeferredRenderingPipeline::processRenderJobRequest()
{
	while(!m_renderJobRequest_queue.empty())
	{
		RenderJobRequest new_jobRequest = m_renderJobRequest_queue.pop();

		std::shared_ptr<Material> material = m_resource_mngr->createMaterial(new_jobRequest.material_path);
		std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(new_jobRequest.mesh_path);

		m_staticMeshes_pass.addRenderJob(RenderJob(new_jobRequest.entity,material,mesh));
	}
}

void DeferredRenderingPipeline::registerStaticMeshComponents()
{
	// Access static mesh components
	auto& staticMesh_data = m_staticMesh_mngr->getData();
	auto& staticMesh_queue = m_staticMesh_mngr->getComponentsQueue();

	// Check for newly added components
	while ( !staticMesh_queue.empty() )
	{
		auto idx = staticMesh_queue.pop();

		StaticMeshComponentManager::Data* component_data = &staticMesh_data[idx];

		std::shared_ptr<Material> material = m_resource_mngr->createMaterial(component_data->material_path);
		//std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path);
		std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path,
										component_data->vertex_data,
										component_data->index_data,
										component_data->vertex_description,
										component_data->mesh_type );


		m_staticMeshes_pass.addRenderJob(RenderJob(component_data->entity,material,mesh));
	}
}

void DeferredRenderingPipeline::registerLightComponents()
{
	// Access point light components
	auto& pointlight_queue = m_light_mngr->getComponentsQueue();

	while( !pointlight_queue.empty() )
	{
		m_active_lightsources.push_back( pointlight_queue.pop() );
	}
}

void DeferredRenderingPipeline::windowSizeCallback(GLFWwindow* window, int width, int height)
{

}

void DeferredRenderingPipeline::windowCloseCallback(GLFWwindow* window)
{

}