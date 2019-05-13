#include "PointlightComponent.hpp"

PointlightComponentManager::PointlightComponentManager(uint size)
{
	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ (2 * static_cast<uint>(sizeof(float))) );
	m_data.buffer = new uint8_t[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.light_colour = (Vec3*)(m_data.entity + size);
	m_data.lumen = (float*)(m_data.light_colour + size);
	m_data.radius = (float*)(m_data.lumen + size);
	
}

PointlightComponentManager::~PointlightComponentManager()
{
	delete[] m_data.buffer;
}

void PointlightComponentManager::reallocate(uint size)
{
	Data new_data;

	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Vec3)
								+ (2 * static_cast<uint>(sizeof(float))) );
	new_data.buffer = new uint8_t[bytes];

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

uint PointlightComponentManager::getComponentCount()
{
	return m_data.used;
}

uint PointlightComponentManager::getIndex(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

std::pair<bool, uint> PointlightComponentManager::getIndex(uint entity_id)
{
	auto search = m_index_map.find(entity_id);

	std::pair<bool, uint> rtn;

	if (search != m_index_map.end())
	{
		rtn.first = true;
		rtn.second = search->second;
	}
	else
	{
		rtn.first = false;
		rtn.second = 0;
	}

	return rtn;
}

Entity PointlightComponentManager::getEntity(uint index)
{
	return m_data.entity[index];
}

Vec3 PointlightComponentManager::getColour(uint index) const
{
	return m_data.light_colour[index];
}

float PointlightComponentManager::getLumen(uint index) const
{
	return m_data.lumen[index];
}

float PointlightComponentManager::getRadius(uint index) const
{
	return m_data.radius[index];
}

void PointlightComponentManager::setColour(uint index, Vec3 colour)
{
	m_data.light_colour[index] = colour;
}

void PointlightComponentManager::setLumen(uint index, float lumen)
{
	m_data.lumen[index] = lumen;
}

void PointlightComponentManager::setRadius(uint index, float radius)
{
	m_data.radius[index] = radius;
}

std::vector<Entity> PointlightComponentManager::getListOfEntities() const
{
	std::vector<Entity> rtn;

	for (uint i = 0; i < m_data.used; i++)
	{
		rtn.push_back(m_data.entity[i]);
	}

	return rtn;
}