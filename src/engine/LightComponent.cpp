#include "LightComponent.hpp"

LightComponentManager::LightComponentManager(uint size)
{
	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ sizeof(float));
	m_data.buffer = new char[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.light_colour = (Vec3*)(m_data.entity + size);
	m_data.light_intensity = (float*)(m_data.light_colour + size);
	
}

LightComponentManager::~LightComponentManager()
{
	delete m_data.buffer;
}

void LightComponentManager::reallocate(uint size)
{
	Data new_data;

	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ sizeof(float));
	new_data.buffer = new char[bytes];

	new_data.used = m_data.used;
	new_data.allocated = size;

	new_data.entity = (Entity*)(m_data.buffer);
	new_data.light_colour = (Vec3*)(m_data.entity + size);
	new_data.light_intensity = (float*)(m_data.light_colour + size);

	std::memcpy(m_data.entity, new_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(m_data.light_colour, new_data.light_intensity, m_data.used * sizeof(Vec3));
	std::memcpy(m_data.light_intensity, new_data.light_intensity, m_data.used * sizeof(float));

	delete m_data.buffer;

	m_data = new_data;
}

void LightComponentManager::addComponent(Entity entity)
{
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_index_map.insert({entity.id(),index});

	m_data.entity[index] = entity;
	m_data.light_colour[index] = Vec3(1.0);
	m_data.light_intensity[index] = 1000.0f;

	m_data.used++;
}

void LightComponentManager::deleteComonent(Entity entity)
{

}

uint LightComponentManager::getIndex(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}