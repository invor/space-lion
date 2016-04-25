#include "DeferredRenderingPipeline.hpp"

DeferredRenderingPipeline::DeferredRenderingPipeline(EntityManager* entity_mngr,
										ResourceManager* resource_mngr,
										TransformComponentManager* transform_mngr,
										CameraComponentManager* camera_mngr,
										PointlightComponentManager* light_mngr,
										SunlightComponentManager* sunligh_mngr,
										AtmosphereComponentManager* atmosphere_mngr,
										StaticMeshComponentManager* staticMesh_mngr,
										VolumeComponentManager* volume_mngr,
										InterfaceMeshComponentManager* interface_mngr)
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
		m_staticMesh_mngr(staticMesh_mngr),
		m_volume_mngr(volume_mngr),
		m_interfaceMesh_mngr(interface_mngr)
{
	transform_mngr->addComponent(m_active_camera,Vec3(0.0f,0.0f,20.0f),Quat(),Vec3(1.0f));
	camera_mngr->addComponent(m_active_camera, 0.1f, 15000.0f, 0.7f);
}

DeferredRenderingPipeline::~DeferredRenderingPipeline()
{
}

void DeferredRenderingPipeline::orderIndependentTransparencyPass()
{
}

void DeferredRenderingPipeline::geometryPass()
{
	glDisable(GL_BLEND);
	m_gBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int width, height;
	height = m_gBuffer->getHeight();
	width = m_gBuffer->getWidth();
	glViewport(0, 0, width, height);

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
			glActiveTexture(GL_TEXTURE0);
			shader.shader_prgm->setUniform("diffuse_tx2D",0);
			material.material->getTextures()[0]->bindTexture();

			glActiveTexture(GL_TEXTURE1);
			shader.shader_prgm->setUniform("specular_tx2D",1);
			material.material->getTextures()[1]->bindTexture();

			glActiveTexture(GL_TEXTURE2);
			shader.shader_prgm->setUniform("roughness_tx2D",2);
			material.material->getTextures()[2]->bindTexture();

			glActiveTexture(GL_TEXTURE3);
			shader.shader_prgm->setUniform("normal_tx2D",3);
			material.material->getTextures()[3]->bindTexture();

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

void DeferredRenderingPipeline::volumePass()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	m_gBuffer->bindColorbuffer(0);

	RenderJobManager::RootNode m_root = m_volume_pass.getRoot();

	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	Mat4x4 proj_matrix = m_camera_mngr->getProjectionMatrix( m_camera_mngr->getIndex(m_active_camera) );

	for(auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();

		// TODO upload camera position
		shader.shader_prgm->setUniform("camera_position", m_transform_mngr->getPosition( m_transform_mngr->getIndex(m_active_camera) ) );

		for(auto& material : shader.materials)
		{
			glActiveTexture(GL_TEXTURE0);
			shader.shader_prgm->setUniform("volume_tx3D",0);
			material.material->getTextures()[0]->bindTexture();

			for(auto& mesh : material.meshes)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;
			
				for(auto& entity : mesh.enities)
				{
					uint transform_index = m_transform_mngr->getIndex(entity);
					Mat4x4 model_matrix = m_transform_mngr->getWorldTransformation(transform_index);

					Vec3 bbox_min = m_volume_mngr->m_data[m_volume_mngr->getIndex(entity)].boundingBox_min;
					Vec3 bbox_max = m_volume_mngr->m_data[m_volume_mngr->getIndex(entity)].boundingBox_max;

					Mat4x4 texture_matrix = glm::scale(glm::inverse(model_matrix), 1.0f/(bbox_max-bbox_min));
					texture_matrix = glm::translate(texture_matrix, -bbox_min);
					shader.shader_prgm->setUniform("texture_matrix", texture_matrix);
			
					Mat4x4 model_view_proj_matrix = proj_matrix * view_matrix * model_matrix;
					//	Mat4x4 model_view_matrix = view_matrix;
					std::string uniform_name("model_view_proj_matrix[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(uniform_name.c_str(), model_view_proj_matrix);
			
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
	m_atmosphere_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_atmosphere_fbo->getWidth(), m_atmosphere_fbo->getHeight());

	glDisable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0);
	m_gBuffer->bindColorbuffer(0);

	AtmosphereComponentManager::Data const * const atmosphere_data = m_atmosphere_mngr->getData();
	uint num_entities = atmosphere_data->used;

	if( num_entities == 0 ) return;

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
		
		glActiveTexture(GL_TEXTURE1);
		atmosphere_data->material[i]->getTextures()[1]->bindTexture();
		shader_prgm->setUniform("rayleigh_inscatter_tx3D",1);

		glActiveTexture(GL_TEXTURE2);
		atmosphere_data->material[i]->getTextures()[2]->bindTexture();
		shader_prgm->setUniform("mie_inscatter_tx3D",2);

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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int width, height;
	glfwGetFramebufferSize(m_active_window, &width, &height);
	glViewport(0, 0, width, height);
	
	glActiveTexture(GL_TEXTURE0);
	m_gBuffer->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	m_gBuffer->bindColorbuffer(1);
	glActiveTexture(GL_TEXTURE2);
	m_gBuffer->bindColorbuffer(2);
	
	glActiveTexture(GL_TEXTURE3);
	m_atmosphere_fbo->bindColorbuffer(0);

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

void DeferredRenderingPipeline::interfacePass()
{
	RenderJobManager::RootNode m_root = m_interface_pass.getRoot();

	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	Mat4x4 proj_matrix = m_camera_mngr->getProjectionMatrix( m_camera_mngr->getIndex(m_active_camera) );

	for(auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();
		shader.shader_prgm->setUniform("projection_matrix", proj_matrix);

		for(auto& material : shader.materials)
		{
			for(auto& mesh : material.meshes)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;

				for(auto& entity : mesh.enities)
				{
					uint transform_index = m_transform_mngr->getIndex(entity);
					Mat4x4 model_matrix = m_transform_mngr->getWorldTransformation(transform_index);
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

void DeferredRenderingPipeline::pickingPass()
{
	RenderJobManager::RootNode m_root = m_picking_pass.getRoot();

	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_mngr->getWorldTransformation( m_transform_mngr->getIndex(m_active_camera) ));
	Mat4x4 proj_matrix = m_camera_mngr->getProjectionMatrix( m_camera_mngr->getIndex(m_active_camera) );

	for(auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();
		shader.shader_prgm->setUniform("projection_matrix", proj_matrix);

		for(auto& material : shader.materials)
		{
			for(auto& mesh : material.meshes)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;

				for(auto& entity : mesh.enities)
				{
					uint transform_index = m_transform_mngr->getIndex(entity);
					Mat4x4 model_matrix = m_transform_mngr->getWorldTransformation(transform_index);
					Mat4x4 model_view_matrix = view_matrix * model_matrix;
					//	Mat4x4 model_view_matrix = view_matrix;
					std::string uniform_name("model_view_matrix[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(uniform_name.c_str(), model_view_matrix);

					std::string id_uniform_name("entity[" + std::to_string(entity.id()) + "]");
					shader.shader_prgm->setUniform(id_uniform_name.c_str(), entity.id());

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

void DeferredRenderingPipeline::processSingleExecTasks()
{
	while ( !m_singleExecution_tasks.empty() )
	{
		std::function<void()> task = m_singleExecution_tasks.pop();
		task();
	}
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

	// TODO Switch to new vertex descriptor style
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
	m_atmosphere_boundingSphere = m_resource_mngr->createIcoSphere(2);

	// Create g-Buffer
	m_gBuffer = std::make_unique<FramebufferObject>(1600,900,true);
	m_gBuffer->createColorAttachment(GL_RGBA16F,GL_RGBA,GL_HALF_FLOAT);
	m_gBuffer->createColorAttachment(GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE);
	m_gBuffer->createColorAttachment(GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE);
	std::cout<<m_gBuffer->getLog()<<std::endl;

	m_atmosphere_fbo = std::make_unique<FramebufferObject>(1600,900,true);
	m_atmosphere_fbo->createColorAttachment(GL_RGBA16F,GL_RGBA,GL_HALF_FLOAT);

	// Set some OpenGL states
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	double t0,t1 = 0.0;

	while (!glfwWindowShouldClose(m_active_window))
	{
		t0 = t1;
		t1 = glfwGetTime();
		double dt = t1 - t0;

		//std::cout<<"Timestep: "<<dt<<std::endl;

		Controls::checkKeyStatus(m_active_window,dt);

		// Register any new gfx related components
		registerStaticMeshComponents();
		registerVolumetricComponents();
		registerInterfaceMeshComponents();
		registerLightComponents();

		// TODO would be nice if method names matched

		// Process new atmoshpere entities
		m_atmosphere_mngr->processNewComponents();

		processSingleExecTasks();


		// Geometry pass
		m_camera_mngr->setCameraAttributes(m_camera_mngr->getIndex(m_active_camera),0.01,10000.0);
		geometryPass();

		// Atmosphere pass
		m_camera_mngr->setCameraAttributes(m_camera_mngr->getIndex(m_active_camera),100.0,6430000.0);
		atmospherePass();
	
		// Lighting pass		
		lightingPass();

		// Volume rendering pass
		m_camera_mngr->setCameraAttributes(m_camera_mngr->getIndex(m_active_camera),0.01,10000.0);
		volumePass();


		//TODO post processing

#ifdef EDITOR_MODE
		// draw interface on top of scene image
		glDisable(GL_BLEND);
		glClear(GL_DEPTH_BUFFER_BIT);
		interfacePass();
#endif

		glfwSwapBuffers(m_active_window);
		glfwPollEvents();
	}

	/* Clear resources while context is still alive */
	m_staticMeshes_pass.clearRenderJobs();
	m_interface_pass.clearRenderJobs();
	m_volume_pass.clearRenderJobs();
	m_resource_mngr->clearLists();
	m_gBuffer.reset();
	m_atmosphere_fbo.reset();
	m_fullscreenQuad.reset();
	m_lighting_prgm.reset();
	m_atmosphere_boundingSphere.reset();

	glfwMakeContextCurrent(NULL);
}

void DeferredRenderingPipeline::addSingleExecutionGpuTask(std::function<void()> task)
{
	m_singleExecution_tasks.push(task);
}

void DeferredRenderingPipeline::setActiveCamera(Entity entity)
{
	m_active_camera = entity;
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

		if( component_data->mesh != nullptr && component_data->material != nullptr )
		{
			m_staticMeshes_pass.addRenderJob(RenderJob(component_data->entity,component_data->material,component_data->mesh));
		}
		else
		{
			std::shared_ptr<Material> material = m_resource_mngr->createMaterial(component_data->material_path);
			//std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path);
			std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path,
											component_data->vertex_data,
											component_data->index_data,
											component_data->vertex_description,
											component_data->mesh_type );

			m_staticMeshes_pass.addRenderJob(RenderJob(component_data->entity,material.get(),mesh.get()));
		}
	}
}

void DeferredRenderingPipeline::registerVolumetricComponents()
{
	// Access volume components
	auto* volume_data = &m_volume_mngr->m_data;
	auto* volume_queue = &m_volume_mngr->m_update_queue;

	while( !volume_queue->empty() )
	{
		auto idx = volume_queue->pop();

		VolumeComponentManager::Data* component_data = &(*volume_data)[idx];

		if( component_data->volume != nullptr && component_data->boundingGeometry != nullptr )
		{
			std::shared_ptr<GLSLProgram> prgm = m_resource_mngr->createShaderProgram({"../resources/shaders/volRen_v.glsl","../resources/shaders/volRen_f.glsl"});
			std::shared_ptr<Material> mtl = m_resource_mngr->createMaterial(component_data->volume->getName()+"_mtl",prgm,{(component_data->volume)});

			m_volume_pass.addRenderJob(RenderJob(component_data->entity,mtl.get(),component_data->boundingGeometry.get()));
		}
		else
		{
			// TODO design new way to create materials s.t. GPU doesn't do an IO

			std::shared_ptr<GLSLProgram> prgm = m_resource_mngr->createShaderProgram({"../resources/shaders/volRen_v.glsl","../resources/shaders/volRen_f.glsl"});
			std::shared_ptr<Texture> volume = m_resource_mngr->createTexture3D(component_data->volume_path,
																				component_data->volume_description.internal_format,
																				component_data->volume_description.width,
																				component_data->volume_description.height,
																				component_data->volume_description.depth,
																				component_data->volume_description.format,
																				component_data->volume_description.type,
																				component_data->volume_data.data());
			(reinterpret_cast<Texture3D*>(volume.get()))->texParameteri<std::vector<std::pair<GLenum,GLenum>>>({std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
																												std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
																												std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
																												std::pair<GLenum,GLenum>(GL_TEXTURE_MAG_FILTER, GL_NEAREST),
																												std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER, GL_NEAREST)});

			std::shared_ptr<Material> mtl = m_resource_mngr->createMaterial(component_data->volume_path+"_mtl",prgm,{volume});

			std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->boundingGeometry_path,
																		component_data->vertex_data,
																		component_data->index_data,
																		component_data->vertex_description,
																		component_data->mesh_type );

			//std::shared_ptr<Mesh> mesh = m_resource_mngr->createBox();

			m_volume_pass.addRenderJob(RenderJob(component_data->entity,mtl.get(),mesh.get()));
		}
	}
}

void DeferredRenderingPipeline::registerInterfaceMeshComponents()
{
	// Access interface mesh components
	auto& interfaceMesh_data = m_interfaceMesh_mngr->getData();
	auto& interfaceMesh_queue = m_interfaceMesh_mngr->getComponentsQueue();

	// Check for newly added components
	while ( !interfaceMesh_queue.empty() )
	{
		auto idx = interfaceMesh_queue.pop();

		InterfaceMeshComponentManager::Data* component_data = &interfaceMesh_data[idx];

		std::shared_ptr<Material> material = m_resource_mngr->createMaterial(component_data->material_path);
		//std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path);
		std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path,
										component_data->vertex_data,
										component_data->index_data,
										component_data->vertex_description,
										component_data->mesh_type );


		m_interface_pass.addRenderJob(RenderJob(component_data->entity,material.get(),mesh.get()));
	}

	auto& update_queue = m_interfaceMesh_mngr->m_updated_components_queue;

	// Check for updated components
	while ( !update_queue.empty() )
	{
		auto idx = update_queue.pop();

		InterfaceMeshComponentManager::Data* component_data = &interfaceMesh_data[idx];

		//std::shared_ptr<Material> material = m_resource_mngr->createMaterial(component_data->material_path);
		//std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path);
		m_resource_mngr->updateMesh(component_data->mesh_path,
										component_data->vertex_data,
										component_data->index_data,
										component_data->vertex_description,
										component_data->mesh_type );


		//m_interface_pass.addRenderJob(RenderJob(component_data->entity,material,mesh));
	}
}

void DeferredRenderingPipeline::registerSelectableCompontens()
{
	// Access select components
	auto& select_data = m_select_mngr->getData();
	auto& select_queue = m_select_mngr->getComponentsQueue();

	// Check for newly added components
	while ( !select_queue.empty() )
	{
		auto idx = select_queue.pop();

		SelectComponentManager::Data* component_data = &select_data[idx];

		std::shared_ptr<Material> material = m_resource_mngr->createMaterial(component_data->material_path);
		//std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path);
		std::shared_ptr<Mesh> mesh = m_resource_mngr->createMesh(component_data->mesh_path,
										component_data->vertex_data,
										component_data->index_data,
										component_data->vertex_description,
										component_data->mesh_type );


		m_picking_pass.addRenderJob(RenderJob(component_data->entity,material.get(),mesh.get()));
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