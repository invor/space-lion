#include "PointlightComponent.hpp"

PointlightComponentManager::PointlightComponentManager(uint size)
{
	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ (2.0 * sizeof(float)) );
	m_data.buffer = new char[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.light_colour = (Vec3*)(m_data.entity + size);
	m_data.lumen = (float*)(m_data.light_colour + size);
	m_data.radius = (float*)(m_data.lumen + size);
	
}

PointlightComponentManager::~PointlightComponentManager()
{
	delete m_data.buffer;
}

void PointlightComponentManager::reallocate(uint size)
{
	Data new_data;

	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ (2.0 * sizeof(float)) );
	new_data.buffer = new char[bytes];

	new_data.used = m_data.used;
	new_data.allocated = size;

	new_data.entity = (Entity*)(new_data.buffer);
	new_data.light_colour = (Vec3*)(new_data.entity + size);
	new_data.lumen = (float*)(new_data.light_colour + size);
	new_data.radius = (float*)(new_data.lumen + size);

	std::memcpy(m_data.entity, new_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(m_data.light_colour, new_data.light_colour, m_data.used * sizeof(Vec3));
	std::memcpy(m_data.lumen, new_data.lumen, m_data.used * sizeof(float));
	std::memcpy(m_data.radius, new_data.radius, m_data.used * sizeof(float));

	delete m_data.buffer;

	m_data = new_data;
}

void PointlightComponentManager::addComponent(Entity entity, Vec3 light_colour, float lumen, float radius)
{
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_index_map.insert({entity.id(),index});

	m_data.entity[index] = entity;
	m_data.light_colour[index] = light_colour;
	m_data.lumen[index] = lumen;
	m_data.radius[index] = radius;

	m_data.used++;
}

void PointlightComponentManager::deleteComonent(Entity entity)
{

}

uint PointlightComponentManager::getIndex(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

const Vec3 PointlightComponentManager::getColour(uint index)
{
	return m_data.light_colour[index];
}

const float PointlightComponentManager::getLumen(uint index)
{
	return m_data.lumen[index];
}

const float PointlightComponentManager::getRadius(uint index)
{
	return m_data.radius[index];
}