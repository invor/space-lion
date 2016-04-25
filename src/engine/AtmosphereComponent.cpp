#include "AtmosphereComponent.hpp"

AtmosphereComponentManager::AtmosphereComponentManager(uint size, ResourceManager* resource_mngr)
	: m_resource_mngr(resource_mngr)
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
}

AtmosphereComponentManager::~AtmosphereComponentManager()
{
	delete m_data.buffer;
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

		std::shared_ptr<GLSLProgram> prgm = m_resource_mngr->createShaderProgram({"../resources/shaders/atmosphere_v.glsl","../resources/shaders/atmosphere_f.glsl"});

		std::shared_ptr<Texture2D> transmittance_table = m_resource_mngr->createTexture2D("transmittance_table_" + m_data.entity[addedComponent_idx].id(),
																						GL_RGBA32F,
																						256,
																						64,
																						GL_RGBA,
																						GL_FLOAT,
																						0 );
		transmittance_table->texParameteri<std::vector<std::pair<GLenum,GLenum>>>(
			{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
				std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) } );

		std::shared_ptr<Texture3D> mie_inscatter_table = m_resource_mngr->createTexture3D("mie_inscatter_table_" + m_data.entity[addedComponent_idx].id(),
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

		std::shared_ptr<Texture3D> rayleigh_inscatter_table  = m_resource_mngr->createTexture3D("rayleigh_inscatter_table_" + m_data.entity[addedComponent_idx].id(),
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

		std::shared_ptr<Texture2D> irradiance_table = m_resource_mngr->createTexture2D("irradience_table_" + m_data.entity[addedComponent_idx].id(),
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
		std::shared_ptr<Material> new_material = m_resource_mngr->createMaterial("atmosphere_" + std::to_string(m_data.entity[addedComponent_idx].id()),
																					prgm,
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

void AtmosphereComponentManager::computeTransmittance(uint index)
{
	auto transmittance_prgm = m_resource_mngr->createShaderProgram({"../resources/shaders/transmittance_c.glsl"});

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
	auto inscatter_single_prgm = m_resource_mngr->createShaderProgram({"../resources/shaders/inscatter_single_c.glsl"});

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