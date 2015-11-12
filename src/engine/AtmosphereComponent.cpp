#include "AtmosphereComponent.hpp"

AtmosphereComponentManager::AtmosphereComponentManager(uint size)
{
	const uint bytes = size * ( sizeof(Entity) +
								2*sizeof(Vec3) +
								4*sizeof(float) );

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

	new_data.entity = (Entity*)(m_data.buffer);
	new_data.beta_r = (Vec3*)(m_data.entity + size);
	new_data.beta_m = (Vec3*)(m_data.beta_r + size);
	new_data.h_r = (float*)(m_data.beta_m + size);
	new_data.h_m = (float*)(m_data.h_r + size);
	new_data.min_altitude = (float*)(m_data.h_m + size);
	new_data.max_altitude = (float*)(m_data.min_altitude + size);

	std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(new_data.beta_r, m_data.beta_r, m_data.used * sizeof(Vec3));
	std::memcpy(new_data.beta_m, m_data.beta_m, m_data.used * sizeof(Vec3));
	std::memcpy(new_data.h_r, m_data.h_r, m_data.used * sizeof(float));
	std::memcpy(new_data.h_m, m_data.h_m, m_data.used * sizeof(float));
	std::memcpy(new_data.min_altitude, m_data.min_altitude, m_data.used * sizeof(float));
	std::memcpy(new_data.max_altitude, m_data.max_altitude, m_data.used * sizeof(float));

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

	//TODO request compute and render jobs
}

const uint AtmosphereComponentManager::getIndex(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}