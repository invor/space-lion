#include "AtmosphereComponent.hpp"

AtmosphereComponentManager::AtmosphereComponentManager(uint size)
{
	const uint bytes = size * ( sizeof(Entity) +
								2*sizeof(Vec3) +
								4*sizeof(float) +
								1*sizeof(Material*) );

	m_data.buffer = new char[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.beta_r = (Vec3*)(m_data.entity + size);
	m_data.beta_m = (Vec3*)(m_data.beta_r + size);
	m_data.h_r = (float*)(m_data.beta_m + size);
	m_data.h_m = (float*)(m_data.h_r + size);
	m_data.min_altitude = (float*)(m_data.h_m + size);
	m_data.max_altitude = (float*)(m_data.min_altitude + size);
	m_data.material = (Material**)(m_data.max_altitude + size);

	// Enqueue GPU task for creating atmosphere proxy mesh and shader prgm
	GCoreComponents::renderingPipeline().addSingleExecutionGpuTask(std::bind(&AtmosphereComponentManager::createGpuResources, this));

	// Enqueue GPU task for rendering atmoshphere
	GCoreComponents::renderingPipeline().addPerFrameAtmosphereGpuTask(std::bind(&AtmosphereComponentManager::draw, this));
}

AtmosphereComponentManager::~AtmosphereComponentManager()
{
	delete m_data.buffer;

	//TODO flag gpu resources for deletion
}

void AtmosphereComponentManager::reallocate(uint size)
{
	Data new_data;
	const uint bytes = size * ( sizeof(Entity) +
								2*sizeof(Vec3) +
								4*sizeof(float) );

	new_data.buffer = new char[bytes];

	new_data.used = m_data.used;
	new_data.allocated = size;

	new_data.entity = (Entity*)(new_data.buffer);
	new_data.beta_r = (Vec3*)(new_data.entity + size);
	new_data.beta_m = (Vec3*)(new_data.beta_r + size);
	new_data.h_r = (float*)(new_data.beta_m + size);
	new_data.h_m = (float*)(new_data.h_r + size);
	new_data.min_altitude = (float*)(new_data.h_m + size);
	new_data.max_altitude = (float*)(new_data.min_altitude + size);
	new_data.material = (Material**)(new_data.max_altitude + size);

	std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(new_data.beta_r, m_data.beta_r, m_data.used * sizeof(Vec3));
	std::memcpy(new_data.beta_m, m_data.beta_m, m_data.used * sizeof(Vec3));
	std::memcpy(new_data.h_r, m_data.h_r, m_data.used * sizeof(float));
	std::memcpy(new_data.h_m, m_data.h_m, m_data.used * sizeof(float));
	std::memcpy(new_data.min_altitude, m_data.min_altitude, m_data.used * sizeof(float));
	std::memcpy(new_data.max_altitude, m_data.max_altitude, m_data.used * sizeof(float));
	std::memcpy(new_data.material, m_data.material, m_data.used * sizeof(Material*));

	delete m_data.buffer;

	m_data = new_data;
}

void AtmosphereComponentManager::addComponent(Entity entity,
					Vec3 beta_r,
					Vec3 beta_m,
					float h_r,
					float h_m,
					float min_altitude,
					float max_altitude)
{
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_index_map.insert({entity.id(),index});
	
	m_data.entity[index] = entity;
	m_data.beta_r[index] = beta_r;
	m_data.beta_m[index] = beta_m;
	m_data.h_r[index] = h_r;
	m_data.h_m[index] = h_m;
	m_data.min_altitude[index] = min_altitude;
	m_data.max_altitude[index] = max_altitude;

	m_data.used++;

	m_addedComponents_queue.push(index);

	// Enqueue GPU tasks for atmosphere computation
	GCoreComponents::renderingPipeline().addSingleExecutionGpuTask( [this, index] { this->createMaterial(index); });
	GCoreComponents::renderingPipeline().addSingleExecutionGpuTask( [this, index] { this->computeTransmittance(index); });
	GCoreComponents::renderingPipeline().addSingleExecutionGpuTask( [this, index] { this->computeInscatterSingle(index); });
	GCoreComponents::renderingPipeline().addSingleExecutionGpuTask( [this, index] { this->computeIrradianceSingle(index); });
}

const uint AtmosphereComponentManager::getIndex(Entity entity) const
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

void AtmosphereComponentManager::processNewComponents()
{
	while(!m_addedComponents_queue.empty())
	{
		uint addedComponent_idx = m_addedComponents_queue.pop();

		std::shared_ptr<GLSLProgram> prgm = GCoreComponents::resourceManager().createShaderProgram({"../resources/shaders/atmosphere_v.glsl","../resources/shaders/atmosphere_f.glsl"});

		std::shared_ptr<Texture2D> transmittance_table = GCoreComponents::resourceManager().createTexture2D("transmittance_table_" + m_data.entity[addedComponent_idx].id(),
																						GL_RGBA32F,
																						256,
																						64,
																						GL_RGBA,
																						GL_FLOAT,
																						0 );
		transmittance_table->texParameteri<std::vector<std::pair<GLenum,GLenum>>>(
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) } );

		std::shared_ptr<Texture3D> mie_inscatter_table = GCoreComponents::resourceManager().createTexture3D("mie_inscatter_table_" + m_data.entity[addedComponent_idx].id(),
																						GL_RGBA32F,
																						256,
																						128,
																						32,
																						GL_RGBA,
																						GL_FLOAT,
																						0);
		mie_inscatter_table->texParameteri<std::vector<std::pair<GLenum,GLenum>>>(
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE) } );

		std::shared_ptr<Texture3D> rayleigh_inscatter_table  = GCoreComponents::resourceManager().createTexture3D("rayleigh_inscatter_table_" + m_data.entity[addedComponent_idx].id(),
																						GL_RGBA32F,
																						256,
																						128,
																						32,
																						GL_RGBA,
																						GL_FLOAT,
																						0);
		rayleigh_inscatter_table->texParameteri<std::vector<std::pair<GLenum,GLenum>>>(
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE) } );

		std::shared_ptr<Texture2D> irradiance_table = GCoreComponents::resourceManager().createTexture2D("irradience_table_" + m_data.entity[addedComponent_idx].id(),
																						GL_RGBA32F,
																						256,
																						64,
																						GL_RGBA,
																						GL_FLOAT,
																						0 );
		irradiance_table->texParameteri<std::vector<std::pair<GLenum,GLenum>>>(
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) } );

		//TODO let the resource_mngr handle the creation!
		std::shared_ptr<Material> new_material = GCoreComponents::resourceManager().createMaterial("atmosphere_" + std::to_string(m_data.entity[addedComponent_idx].id()), prgm,
																					{ transmittance_table,rayleigh_inscatter_table,mie_inscatter_table,irradiance_table });

		m_data.material[addedComponent_idx] = (Material*) new_material.get();

		computeTransmittance(addedComponent_idx);
		computeInscatterSingle(addedComponent_idx);
		computeIrradianceSingle(addedComponent_idx);
	}
}

AtmosphereComponentManager::Data const * const AtmosphereComponentManager::getData() const
{
	return &m_data;
}

void AtmosphereComponentManager::createGpuResources()
{
	m_atmosphere_prgm = GCoreComponents::resourceManager().createShaderProgram({ "../resources/shaders/atmosphere_v.glsl","../resources/shaders/atmosphere_f.glsl" });
	m_atmosphere_proxySphere = GCoreComponents::resourceManager().createIcoSphere(2).get();
}

void AtmosphereComponentManager::createMaterial(uint index)
{
	std::shared_ptr<Texture2D> transmittance_table = GCoreComponents::resourceManager().createTexture2D("transmittance_table_" + m_data.entity[index].id(),
																						GL_RGBA32F,
																						256,
																						64,
																						GL_RGBA,
																						GL_FLOAT,
																						0);
	transmittance_table->texParameteri<std::vector<std::pair<GLenum, GLenum>>>(
	{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
		std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) });

	std::shared_ptr<Texture3D> mie_inscatter_table = GCoreComponents::resourceManager().createTexture3D("mie_inscatter_table_" + m_data.entity[index].id(),
																						GL_RGBA32F,
																						256,
																						128,
																						32,
																						GL_RGBA,
																						GL_FLOAT,
																						0);
	mie_inscatter_table->texParameteri<std::vector<std::pair<GLenum, GLenum>>>(
	{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
		std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
		std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE) });

	std::shared_ptr<Texture3D> rayleigh_inscatter_table = GCoreComponents::resourceManager().createTexture3D("rayleigh_inscatter_table_" + m_data.entity[index].id(),
																							GL_RGBA32F,
																							256,
																							128,
																							32,
																							GL_RGBA,
																							GL_FLOAT,
																							0);
	rayleigh_inscatter_table->texParameteri<std::vector<std::pair<GLenum, GLenum>>>(
	{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
		std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
		std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE) });

	std::shared_ptr<Texture2D> irradiance_table = GCoreComponents::resourceManager().createTexture2D("irradience_table_" + m_data.entity[index].id(),
																					GL_RGBA32F,
																					256,
																					64,
																					GL_RGBA,
																					GL_FLOAT,
																					0);
	irradiance_table->texParameteri<std::vector<std::pair<GLenum, GLenum>>>(
	{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
		std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) });

	//TODO let the resource_mngr handle the creation!
	std::shared_ptr<Material> new_material = GCoreComponents::resourceManager().createMaterial("atmosphere_" + std::to_string(m_data.entity[index].id()), m_atmosphere_prgm,
																				{ transmittance_table,rayleigh_inscatter_table,mie_inscatter_table,irradiance_table });

	m_data.material[index] = (Material*)new_material.get();
}

void AtmosphereComponentManager::computeTransmittance(uint index)
{
	auto transmittance_prgm = GCoreComponents::resourceManager().createShaderProgram({"../resources/shaders/transmittance_c.glsl"});

	transmittance_prgm->use();

	glBindImageTexture(0,m_data.material[index]->getTextures()[0]->getHandle(),0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	transmittance_prgm->setUniform("transmittance_tx2D",0);

	transmittance_prgm->setUniform("min_altitude",m_data.min_altitude[index]);
	transmittance_prgm->setUniform("max_altitude",m_data.max_altitude[index]);
	transmittance_prgm->setUniform("beta_r",m_data.beta_r[index]);
	transmittance_prgm->setUniform("beta_m",m_data.beta_m[index]);
	transmittance_prgm->setUniform("h_m",m_data.h_m[index]);
	transmittance_prgm->setUniform("h_r",m_data.h_r[index]);

	transmittance_prgm->dispatchCompute(256,64,1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void AtmosphereComponentManager::computeInscatterSingle(uint index)
{
	auto inscatter_single_prgm = GCoreComponents::resourceManager().createShaderProgram({"../resources/shaders/inscatter_single_c.glsl"});

	inscatter_single_prgm->use();

	glBindImageTexture(0,m_data.material[index]->getTextures()[1]->getHandle(),0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	inscatter_single_prgm->setUniform("rayleigh_inscatter_tx3D",0);

	glBindImageTexture(1,m_data.material[index]->getTextures()[2]->getHandle(),0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	inscatter_single_prgm->setUniform("mie_inscatter_tx3D",1);

	glActiveTexture(GL_TEXTURE2);
	inscatter_single_prgm->setUniform("transmittance_tx2D",2);
	m_data.material[index]->getTextures()[0]->bindTexture();
	glDisable(GL_TEXTURE_2D);

	inscatter_single_prgm->setUniform("min_altitude",m_data.min_altitude[index]);
	inscatter_single_prgm->setUniform("max_altitude",m_data.max_altitude[index]);
	inscatter_single_prgm->setUniform("beta_r",m_data.beta_r[index]);
	inscatter_single_prgm->setUniform("beta_m",m_data.beta_m[index]);
	inscatter_single_prgm->setUniform("h_r",m_data.h_r[index]);
	inscatter_single_prgm->setUniform("h_m",m_data.h_m[index]);

	inscatter_single_prgm->dispatchCompute(32,128,32);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void AtmosphereComponentManager::computeIrradianceSingle(uint index)
{
}

void AtmosphereComponentManager::draw()
{
	Entity active_camera = GCoreComponents::renderingPipeline().getActiveCamera();

	glDisable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0);
	GCoreComponents::renderingPipeline().getGBuffer()->bindColorbuffer(0);
	//m_gBuffer->bindColorbuffer(0);

	// TODO: Try to remove redundancy of getting this information twice during rendering a single frame
	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_camera)));
	Mat4x4 proj_matrix = GCoreComponents::cameraManager().getProjectionMatrix(GCoreComponents::cameraManager().getIndex(active_camera));

	m_atmosphere_prgm->use();
	m_atmosphere_prgm->setUniform("normal_depth_tx2D", 0);
	m_atmosphere_prgm->setUniform("projection_matrix", proj_matrix);
	m_atmosphere_prgm->setUniform("view_matrix", view_matrix);
	m_atmosphere_prgm->setUniform("camera_position", GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(active_camera)));

	Vec3 camera_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(active_camera));
	Vec3 atmosphere_center_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(m_data.entity[0]));

	int sun_count = GCoreComponents::sunlightManager().getData()->used;
	for (int i = 0; i<sun_count; i++)
	{
		std::string sun_direction_uniform("suns[" + std::to_string(i) + "].sun_direction");
		std::string sun_luminance_uniform("suns[" + std::to_string(i) + "].sun_luminance");

		Vec3 light_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(GCoreComponents::sunlightManager().getData()->entity[i]));
		m_atmosphere_prgm->setUniform(sun_direction_uniform.c_str(), (light_position - camera_position));

		// Compute luminance of sun (simplified to a pointlight) just before it hits the atmosphere
		float sun_luminance = GCoreComponents::sunlightManager().getLumen(i);
		float distance = std::sqrt(
			(light_position - atmosphere_center_position).x*(light_position - atmosphere_center_position).x +
			(light_position - atmosphere_center_position).y*(light_position - atmosphere_center_position).y +
			(light_position - atmosphere_center_position).z*(light_position - atmosphere_center_position).z) - m_data.max_altitude[0];
		sun_luminance = sun_luminance / (4.0f * 3.14f * std::pow(distance, 2.0f));

		m_atmosphere_prgm->setUniform(sun_luminance_uniform.c_str(), sun_luminance);
		//std::cout<<"Luminance: "<<sun_luminance<<std::endl;
		//std::cout<<"Distance: "<< distance <<std::endl;
	}
	m_atmosphere_prgm->setUniform("sun_count", sun_count);

	/*	Draw all entities instanced */
	int instance_counter = 0;
	std::string atmosphere_center_uniform;
	std::string max_altitude_uniform;
	std::string min_altitude_uniform;
	std::string model_uniform;


	uint num_entities = m_data.used;

	for (uint i = 0; i<num_entities; i++)
	{
		uint transform_index = GCoreComponents::transformManager().getIndex(m_data.entity[i]);
		Mat4x4 model_matrix = GCoreComponents::transformManager().getWorldTransformation(transform_index);
		model_uniform = ("model_matrix[" + std::to_string(instance_counter) + "]");
		m_atmosphere_prgm->setUniform(model_uniform.c_str(), model_matrix);

		max_altitude_uniform = ("max_altitude[" + std::to_string(instance_counter) + "]");
		m_atmosphere_prgm->setUniform(max_altitude_uniform.c_str(), m_data.max_altitude[i]);

		min_altitude_uniform = ("min_altitude[" + std::to_string(instance_counter) + "]");
		m_atmosphere_prgm->setUniform(min_altitude_uniform.c_str(), m_data.min_altitude[i]);

		atmosphere_center_uniform = ("atmosphere_center[" + std::to_string(instance_counter) + "]");
		m_atmosphere_prgm->setUniform(atmosphere_center_uniform.c_str(), GCoreComponents::transformManager().getPosition(transform_index));

		glActiveTexture(GL_TEXTURE1);
		m_data.material[i]->getTextures()[1]->bindTexture();
		m_atmosphere_prgm->setUniform("rayleigh_inscatter_tx3D", 1);

		glActiveTexture(GL_TEXTURE2);
		m_data.material[i]->getTextures()[2]->bindTexture();
		m_atmosphere_prgm->setUniform("mie_inscatter_tx3D", 2);

		instance_counter++;

		if (instance_counter == 128)
		{
			m_atmosphere_proxySphere->draw(instance_counter);
			instance_counter = 0;
		}
	}

	m_atmosphere_proxySphere->draw(instance_counter);
}